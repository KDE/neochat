// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatconnection.h"

#include <QImageReader>
#include <QJsonDocument>

#include "controller.h"
#include "jobs/neochatchangepasswordjob.h"
#include "jobs/neochatdeactivateaccountjob.h"
#include "linkpreviewer.h"
#include "neochatconfig.h"
#include "neochatroom.h"
#include "roommanager.h"
#include "spacehierarchycache.h"

#include <Quotient/connection.h>
#include <Quotient/csapi/cross_signing.h>
#include <Quotient/e2ee/cryptoutils.h>
#include <Quotient/e2ee/e2ee_common.h>
#include <Quotient/jobs/basejob.h>
#include <Quotient/quotient_common.h>
#include <qt6keychain/keychain.h>

#include <olm/pk.h>

#include <KLocalizedString>

#include <Quotient/csapi/content-repo.h>
#include <Quotient/csapi/profile.h>
#include <Quotient/database.h>
#include <Quotient/jobs/downloadfilejob.h>
#include <Quotient/qt_connection_util.h>
#include <Quotient/room.h>
#include <Quotient/settings.h>
#include <Quotient/user.h>

#ifdef HAVE_KUNIFIEDPUSH
#include <QCoroNetwork>
#include <Quotient/csapi/pusher.h>
#include <Quotient/networkaccessmanager.h>
#endif

using namespace Quotient;
using namespace Qt::StringLiterals;

NeoChatConnection::NeoChatConnection(QObject *parent)
    : Connection(parent)
    , m_threePIdModel(new ThreePIdModel(this))
{
    connectSignals();
}

NeoChatConnection::NeoChatConnection(const QUrl &server, QObject *parent)
    : Connection(server, parent)
    , m_threePIdModel(new ThreePIdModel(this))
{
    connectSignals();
}

void NeoChatConnection::connectSignals()
{
    connect(this, &NeoChatConnection::accountDataChanged, this, [this](const QString &type) {
        if (type == QLatin1String("org.kde.neochat.account_label")) {
            Q_EMIT labelChanged();
        }
        if (type == QLatin1String("m.identity_server")) {
            Q_EMIT identityServerChanged();
        }
    });
    connect(this, &NeoChatConnection::syncDone, this, [this] {
        setIsOnline(true);
    });
    connect(this, &NeoChatConnection::networkError, this, [this]() {
        setIsOnline(false);
    });
    connect(this, &NeoChatConnection::requestFailed, this, [this](BaseJob *job) {
        if (job->error() == BaseJob::UserConsentRequired) {
            Q_EMIT userConsentRequired(job->errorUrl());
        }
    });
    connect(this, &NeoChatConnection::requestFailed, this, [](BaseJob *job) {
        if (dynamic_cast<DownloadFileJob *>(job) && job->jsonData()["errcode"_ls].toString() == "M_TOO_LARGE"_ls) {
            RoomManager::instance().warning(i18n("File too large to download."), i18n("Contact your matrix server administrator for support."));
        }
    });
    connect(this, &NeoChatConnection::directChatsListChanged, this, [this](DirectChatsMap additions, DirectChatsMap removals) {
        Q_EMIT directChatInvitesChanged();
        for (const auto &chatId : additions) {
            if (const auto chat = room(chatId)) {
                connect(chat, &Room::unreadStatsChanged, this, [this]() {
                    refreshBadgeNotificationCount();
                    Q_EMIT directChatNotificationsChanged();
                    Q_EMIT directChatsHaveHighlightNotificationsChanged();
                });
            }
        }
        for (const auto &chatId : removals) {
            if (const auto chat = room(chatId)) {
                disconnect(chat, &Room::unreadStatsChanged, this, nullptr);
            }
        }
    });
    connect(this, &NeoChatConnection::joinedRoom, this, [this](Room *room) {
        if (room->isDirectChat()) {
            connect(room, &Room::unreadStatsChanged, this, [this]() {
                Q_EMIT directChatNotificationsChanged();
                Q_EMIT directChatsHaveHighlightNotificationsChanged();
            });
        }
        connect(room, &Room::unreadStatsChanged, this, [this]() {
            refreshBadgeNotificationCount();
            Q_EMIT homeNotificationsChanged();
            Q_EMIT homeHaveHighlightNotificationsChanged();
        });
    });
    connect(this, &NeoChatConnection::leftRoom, this, [this](Room *room, Room *prev) {
        Q_UNUSED(room)
        if (prev && prev->isDirectChat()) {
            Q_EMIT directChatInvitesChanged();
            Q_EMIT directChatNotificationsChanged();
            Q_EMIT directChatsHaveHighlightNotificationsChanged();
        }
        refreshBadgeNotificationCount();
        Q_EMIT homeNotificationsChanged();
        Q_EMIT homeHaveHighlightNotificationsChanged();
    });

    connect(&SpaceHierarchyCache::instance(), &SpaceHierarchyCache::spaceHierarchyChanged, this, [this]() {
        refreshBadgeNotificationCount();
        Q_EMIT homeNotificationsChanged();
        Q_EMIT homeHaveHighlightNotificationsChanged();
    });
}

int NeoChatConnection::badgeNotificationCount() const
{
    return m_badgeNotificationCount;
}

void NeoChatConnection::refreshBadgeNotificationCount()
{
    int count = 0;
    for (const auto &r : allRooms()) {
        if (const auto room = static_cast<NeoChatRoom *>(r)) {
            count += room->contextAwareNotificationCount();
        }
    }

    if (count != m_badgeNotificationCount) {
        m_badgeNotificationCount = count;
        Q_EMIT badgeNotificationCountChanged(this, m_badgeNotificationCount);
    }
}

void NeoChatConnection::logout(bool serverSideLogout)
{
    SettingsGroup(QStringLiteral("Accounts")).remove(userId());

    QKeychain::DeletePasswordJob job(qAppName());
    job.setAutoDelete(true);
    job.setKey(userId());
    QEventLoop loop;
    QKeychain::DeletePasswordJob::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (!serverSideLogout) {
        return;
    }
    Connection::logout();
}

bool NeoChatConnection::setAvatar(const QUrl &avatarSource)
{
    QString decoded = avatarSource.path();
    if (decoded.isEmpty()) {
        callApi<SetAvatarUrlJob>(user()->id(), avatarSource);
        return true;
    }
    if (QImageReader(decoded).read().isNull()) {
        return false;
    } else {
        return user()->setAvatar(decoded);
    }
}

QVariantList NeoChatConnection::getSupportedRoomVersions() const
{
    const auto &roomVersions = availableRoomVersions();
    QVariantList supportedRoomVersions;
    for (const auto &v : roomVersions) {
        QVariantMap roomVersionMap;
        roomVersionMap.insert("id"_ls, v.id);
        roomVersionMap.insert("status"_ls, v.status);
        roomVersionMap.insert("isStable"_ls, v.isStable());
        supportedRoomVersions.append(roomVersionMap);
    }
    return supportedRoomVersions;
}

void NeoChatConnection::changePassword(const QString &currentPassword, const QString &newPassword)
{
    auto job = callApi<NeochatChangePasswordJob>(newPassword, false);
    connect(job, &BaseJob::result, this, [this, job, currentPassword, newPassword] {
        if (job->error() == 103) {
            QJsonObject replyData = job->jsonData();
            QJsonObject authData;
            authData["session"_ls] = replyData["session"_ls];
            authData["password"_ls] = currentPassword;
            authData["type"_ls] = "m.login.password"_ls;
            authData["user"_ls] = user()->id();
            QJsonObject identifier = {{"type"_ls, "m.id.user"_ls}, {"user"_ls, user()->id()}};
            authData["identifier"_ls] = identifier;
            NeochatChangePasswordJob *innerJob = callApi<NeochatChangePasswordJob>(newPassword, false, authData);
            connect(innerJob, &BaseJob::success, this, [this]() {
                Q_EMIT passwordStatus(PasswordStatus::Success);
            });
            connect(innerJob, &BaseJob::failure, this, [innerJob, this]() {
                Q_EMIT passwordStatus(innerJob->jsonData()["errcode"_ls] == "M_FORBIDDEN"_ls ? PasswordStatus::Wrong : PasswordStatus::Other);
            });
        }
    });
}

void NeoChatConnection::setLabel(const QString &label)
{
    QJsonObject json{
        {"account_label"_ls, label},
    };
    setAccountData("org.kde.neochat.account_label"_ls, json);
    Q_EMIT labelChanged();
}

QString NeoChatConnection::label() const
{
    return accountDataJson("org.kde.neochat.account_label"_ls)["account_label"_ls].toString();
}

void NeoChatConnection::deactivateAccount(const QString &password)
{
    auto job = callApi<NeoChatDeactivateAccountJob>();
    connect(job, &BaseJob::result, this, [this, job, password] {
        if (job->error() == 103) {
            QJsonObject replyData = job->jsonData();
            QJsonObject authData;
            authData["session"_ls] = replyData["session"_ls];
            authData["password"_ls] = password;
            authData["type"_ls] = "m.login.password"_ls;
            authData["user"_ls] = user()->id();
            QJsonObject identifier = {{"type"_ls, "m.id.user"_ls}, {"user"_ls, user()->id()}};
            authData["identifier"_ls] = identifier;
            auto innerJob = callApi<NeoChatDeactivateAccountJob>(authData);
            connect(innerJob, &BaseJob::success, this, [this]() {
                logout(false);
            });
        }
    });
}

ThreePIdModel *NeoChatConnection::threePIdModel() const
{
    return m_threePIdModel;
}

bool NeoChatConnection::hasIdentityServer() const
{
    if (!hasAccountData(QLatin1String("m.identity_server"))) {
        return false;
    }

    const auto url = accountData(QLatin1String("m.identity_server"))->contentPart<QUrl>(QLatin1String("base_url"));
    if (!url.isEmpty()) {
        return true;
    }
    return false;
}

QUrl NeoChatConnection::identityServer() const
{
    if (!hasAccountData(QLatin1String("m.identity_server"))) {
        return {};
    }

    const auto url = accountData(QLatin1String("m.identity_server"))->contentPart<QUrl>(QLatin1String("base_url"));
    if (!url.isEmpty()) {
        return url;
    }
    return {};
}

QString NeoChatConnection::identityServerUIString() const
{
    if (!hasIdentityServer()) {
        return i18nc("@info", "No identity server configured");
    }

    return identityServer().toString();
}

void NeoChatConnection::createRoom(const QString &name, const QString &topic, const QString &parent, bool setChildParent)
{
    QList<CreateRoomJob::StateEvent> initialStateEvents;
    if (!parent.isEmpty()) {
        initialStateEvents.append(CreateRoomJob::StateEvent{
            "m.space.parent"_ls,
            QJsonObject{
                {"canonical"_ls, true},
                {"via"_ls, QJsonArray{domain()}},
            },
            parent,
        });
    }

    const auto job = Connection::createRoom(Connection::PublishRoom, QString(), name, topic, QStringList(), {}, {}, {}, initialStateEvents);
    if (!parent.isEmpty()) {
        connect(job, &Quotient::CreateRoomJob::success, this, [this, parent, setChildParent, job]() {
            if (setChildParent) {
                if (auto parentRoom = room(parent)) {
                    parentRoom->setState(QLatin1String("m.space.child"), job->roomId(), QJsonObject{{QLatin1String("via"), QJsonArray{domain()}}});
                }
            }
        });
    }
    connect(job, &CreateRoomJob::failure, this, [job] {
        Q_EMIT Controller::instance().errorOccured(i18n("Room creation failed: %1", job->errorString()), {});
    });
    connectSingleShot(this, &Connection::newRoom, this, [](Room *room) {
        RoomManager::instance().resolveResource(room->id());
    });
}

void NeoChatConnection::createSpace(const QString &name, const QString &topic, const QString &parent, bool setChildParent)
{
    QList<CreateRoomJob::StateEvent> initialStateEvents;
    if (!parent.isEmpty()) {
        initialStateEvents.append(CreateRoomJob::StateEvent{
            "m.space.parent"_ls,
            QJsonObject{
                {"canonical"_ls, true},
                {"via"_ls, QJsonArray{domain()}},
            },
            parent,
        });
    }

    const auto job = Connection::createRoom(Connection::UnpublishRoom, {}, name, topic, {}, {}, {}, false, initialStateEvents, {}, QJsonObject{{"type"_ls, "m.space"_ls}});
    if (!parent.isEmpty()) {
        connect(job, &Quotient::CreateRoomJob::success, this, [this, parent, setChildParent, job]() {
            if (setChildParent) {
                if (auto parentRoom = room(parent)) {
                    parentRoom->setState(QLatin1String("m.space.child"), job->roomId(), QJsonObject{{QLatin1String("via"), QJsonArray{domain()}}});
                }
            }
        });
    }
    connect(job, &CreateRoomJob::failure, this, [job] {
        Q_EMIT Controller::instance().errorOccured(i18n("Space creation failed: %1", job->errorString()), {});
    });
    connectSingleShot(this, &Connection::newRoom, this, [](Room *room) {
        RoomManager::instance().resolveResource(room->id());
    });
}

bool NeoChatConnection::directChatExists(Quotient::User *user)
{
    return directChats().contains(user);
}

void NeoChatConnection::openOrCreateDirectChat(const QString &userId)
{
    if (auto user = this->user(userId)) {
        openOrCreateDirectChat(user);
    } else {
        qWarning() << "openOrCreateDirectChat: Couldn't get user object for ID " << userId << ", unable to open/request direct chat.";
    }
}

void NeoChatConnection::openOrCreateDirectChat(User *user)
{
    const auto existing = directChats();

    if (existing.contains(user)) {
        const auto room = this->room(existing.value(user));
        if (room) {
            RoomManager::instance().resolveResource(room->id());
            return;
        }
    }
    requestDirectChat(user->id());
}

qsizetype NeoChatConnection::directChatNotifications() const
{
    qsizetype notifications = 0;
    QStringList added; // The same ID can be in the list multiple times.
    for (const auto &chatId : directChats()) {
        if (!added.contains(chatId)) {
            if (const auto chat = room(chatId)) {
                notifications += dynamic_cast<NeoChatRoom *>(chat)->contextAwareNotificationCount();
                added += chatId;
            }
        }
    }
    return notifications;
}

bool NeoChatConnection::directChatsHaveHighlightNotifications() const
{
    for (const auto &childId : directChats()) {
        if (const auto child = static_cast<NeoChatRoom *>(room(childId))) {
            if (child->highlightCount() > 0) {
                return true;
            }
        }
    }
    return false;
}

qsizetype NeoChatConnection::homeNotifications() const
{
    qsizetype notifications = 0;
    QStringList added;
    const auto &spaceHierarchyCache = SpaceHierarchyCache::instance();
    for (const auto &r : allRooms()) {
        if (const auto room = static_cast<NeoChatRoom *>(r)) {
            if (!added.contains(room->id()) && !room->isDirectChat() && !spaceHierarchyCache.isChild(room->id())) {
                notifications += dynamic_cast<NeoChatRoom *>(room)->contextAwareNotificationCount();
                added += room->id();
            }
        }
    }
    return notifications;
}

bool NeoChatConnection::homeHaveHighlightNotifications() const
{
    const auto &spaceHierarchyCache = SpaceHierarchyCache::instance();
    for (const auto &r : allRooms()) {
        if (const auto room = static_cast<NeoChatRoom *>(r)) {
            if (!room->isDirectChat() && !spaceHierarchyCache.isChild(room->id()) && room->highlightCount() > 0) {
                return true;
            }
        }
    }
    return false;
}

bool NeoChatConnection::directChatInvites() const
{
    auto inviteRooms = rooms(JoinState::Invite);
    for (const auto inviteRoom : inviteRooms) {
        if (inviteRoom->isDirectChat()) {
            return true;
        }
    }
    return false;
}

QCoro::Task<void> NeoChatConnection::setupPushNotifications(QString endpoint)
{
#ifdef HAVE_KUNIFIEDPUSH
    QUrl gatewayEndpoint(endpoint);
    gatewayEndpoint.setPath(QStringLiteral("/_matrix/push/v1/notify"));

    QNetworkRequest checkGateway(gatewayEndpoint);
    auto reply = co_await NetworkAccessManager::instance()->get(checkGateway);

    // We want to check if this UnifiedPush server has a Matrix gateway
    // This is because Matrix does not natively support UnifiedPush
    const auto &replyJson = QJsonDocument::fromJson(reply->readAll()).object();

    if (replyJson["unifiedpush"_L1]["gateway"_L1].toString() == QStringLiteral("matrix")) {
        callApi<PostPusherJob>(endpoint,
                               QStringLiteral("http"),
                               QStringLiteral("org.kde.neochat"),
                               QStringLiteral("NeoChat"),
                               deviceId(),
                               QString(), // profileTag is intentionally left empty for now, it's optional
                               QStringLiteral("en-US"),
                               PostPusherJob::PusherData{QUrl::fromUserInput(gatewayEndpoint.toString()), QStringLiteral(" ")},
                               false);

        qInfo() << "Registered for push notifications";
    } else {
        qWarning() << "There's no gateway, not setting up push notifications.";
    }
#else
    co_return;
#endif
}

QString NeoChatConnection::deviceKey() const
{
    return edKeyForUserDevice(userId(), deviceId());
}

QString NeoChatConnection::encryptionKey() const
{
    auto query = database()->prepareQuery(QStringLiteral("SELECT curveKey FROM tracked_devices WHERE matrixId=:matrixId AND deviceid=:deviceId LIMIT 1;"));
    query.bindValue(QStringLiteral(":matrixId"), userId());
    query.bindValue(QStringLiteral(":deviceId"), deviceId());
    database()->execute(query);
    if (!query.next()) {
        return {};
    }
    return query.value(0).toString();
}

bool NeoChatConnection::isOnline() const
{
    return m_isOnline;
}

void NeoChatConnection::setIsOnline(bool isOnline)
{
    if (isOnline == m_isOnline) {
        return;
    }
    m_isOnline = isOnline;
    Q_EMIT isOnlineChanged();
}

QString NeoChatConnection::accountDataJsonString(const QString &type) const
{
    return QString::fromUtf8(QJsonDocument(accountDataJson(type)).toJson());
}

LinkPreviewer *NeoChatConnection::previewerForLink(const QUrl &link)
{
    if (!NeoChatConfig::showLinkPreview()) {
        return nullptr;
    }

    auto previewer = m_linkPreviewers.value(link, nullptr);
    if (previewer != nullptr) {
        return previewer;
    }

    previewer = new LinkPreviewer(link, this);
    m_linkPreviewers[link] = previewer;
    return previewer;
}

void NeoChatConnection::setupCrossSigningKeys(const QString &password)
{
    auto masterKeyPrivate = getRandom<32>();
    auto masterKeyContext = makeCStruct(olm_pk_signing, olm_pk_signing_size, olm_clear_pk_signing);
    QByteArray masterKeyPublic(olm_pk_signing_public_key_length(), 0);
    olm_pk_signing_key_from_seed(masterKeyContext.get(),
                                 masterKeyPublic.data(),
                                 masterKeyPublic.length(),
                                 masterKeyPrivate.data(),
                                 masterKeyPrivate.viewAsByteArray().length());

    auto selfSigningKeyPrivate = getRandom<32>();
    auto selfSigningKeyContext = makeCStruct(olm_pk_signing, olm_pk_signing_size, olm_clear_pk_signing);
    QByteArray selfSigningKeyPublic(olm_pk_signing_public_key_length(), 0);
    olm_pk_signing_key_from_seed(selfSigningKeyContext.get(),
                                 selfSigningKeyPublic.data(),
                                 selfSigningKeyPublic.length(),
                                 selfSigningKeyPrivate.data(),
                                 selfSigningKeyPrivate.viewAsByteArray().length());

    auto userSigningKeyPrivate = getRandom<32>();
    auto userSigningKeyContext = makeCStruct(olm_pk_signing, olm_pk_signing_size, olm_clear_pk_signing);
    QByteArray userSigningKeyPublic(olm_pk_signing_public_key_length(), 0);
    olm_pk_signing_key_from_seed(userSigningKeyContext.get(),
                                 userSigningKeyPublic.data(),
                                 userSigningKeyPublic.length(),
                                 userSigningKeyPrivate.data(),
                                 userSigningKeyPrivate.viewAsByteArray().length());

    database()->storeEncrypted("m.cross_signing.master"_ls, masterKeyPrivate.viewAsByteArray());
    database()->storeEncrypted("m.cross_signing.self_signing"_ls, selfSigningKeyPrivate.viewAsByteArray());
    database()->storeEncrypted("m.cross_signing.user_signing"_ls, userSigningKeyPrivate.viewAsByteArray());

    auto masterKey = CrossSigningKey{
        .userId = userId(),
        .usage = {"master"_ls},
        .keys = {{"ed25519:"_ls + QString::fromLatin1(masterKeyPublic), QString::fromLatin1(masterKeyPublic)}},
        .signatures = {},
    };
    auto selfSigningKey = CrossSigningKey{
        .userId = userId(),
        .usage = {"self_signing"_ls},
        .keys = {{"ed25519:"_ls + QString::fromLatin1(selfSigningKeyPublic), QString::fromLatin1(selfSigningKeyPublic)}},
    };
    auto userSigningKey = CrossSigningKey{
        .userId = userId(),
        .usage = {"user_signing"_ls},
        .keys = {{"ed25519:"_ls + QString::fromLatin1(userSigningKeyPublic), QString::fromLatin1(userSigningKeyPublic)}},

    };

    auto selfSigningKeyJson = toJson(selfSigningKey);
    selfSigningKeyJson.remove("signatures"_ls);
    selfSigningKey.signatures = QJsonObject{
        {userId(),
         QJsonObject{{"ed25519:"_ls + QString::fromLatin1(masterKeyPublic),
                      QString::fromLatin1(sign(masterKeyPrivate.viewAsByteArray(), QJsonDocument(selfSigningKeyJson).toJson(QJsonDocument::Compact)))}}}};
    auto userSigningKeyJson = toJson(userSigningKey);
    userSigningKeyJson.remove("signatures"_ls);
    userSigningKey.signatures = QJsonObject{
        {userId(),
         QJsonObject{{"ed25519:"_ls + QString::fromLatin1(masterKeyPublic),
                      QString::fromLatin1(sign(masterKeyPrivate.viewAsByteArray(), QJsonDocument(userSigningKeyJson).toJson(QJsonDocument::Compact)))}}}};

    auto job = callApi<UploadCrossSigningKeysJob>(masterKey, selfSigningKey, userSigningKey, std::nullopt);
    connect(job, &BaseJob::failure, this, [this, masterKey, selfSigningKey, userSigningKey, password](const auto &job) {
        callApi<UploadCrossSigningKeysJob>(masterKey,
                                           selfSigningKey,
                                           userSigningKey,
                                           AuthenticationData{
                                               .type = "m.login.password"_ls,
                                               .session = job->jsonData()["session"_ls].toString(),
                                               .authInfo =
                                                   QVariantHash{
                                                       {"password"_ls, password},
                                                       {"identifier"_ls,
                                                        QJsonObject{
                                                            {"type"_ls, "m.id.user"_ls},
                                                            {"user"_ls, userId()},
                                                        }},
                                                   },

                                           })
            .then([this](const auto &job) {
                auto key = getRandom(32);
                QByteArray data = QByteArrayLiteral("\x8B\x01") + viewAsByteArray(key);
                data.append(std::accumulate(data.cbegin(), data.cend(), uint8_t{0}, std::bit_xor<>()));
                data = base58Encode(data);
                QList<QString> groups;
                for (auto i = 0; i < data.size() / 4; i++) {
                    groups += QString::fromLatin1(data.mid(i * 4, i * 4 + 4));
                }
                auto formatted = groups.join(QStringLiteral(" "));

                auto iv = getRandom(16);
                data[8] &= ~(1 << 7); // Byte 63 needs to be set to 0

                const auto &testKeys = hkdfSha256(byte_view_t<>(key).subspan<0, DefaultPbkdf2KeyLength>(), zeroes<32>(), {});
                if (!testKeys.has_value()) {
                    qWarning() << "SSSS: Failed to calculate HKDF";
                    // Q_EMIT error(DecryptionError);
                    return;
                }
                const auto &encrypted = aesCtr256Encrypt(zeroedByteArray(), testKeys.value().aes(), asCBytes<AesBlockSize>(iv));
                if (!encrypted.has_value()) {
                    qWarning() << "SSSS: Failed to encrypt test keys";
                    // emit error(DecryptionError);
                    return;
                }
                const auto &result = hmacSha256(testKeys.value().mac(), encrypted.value());
                if (!result.has_value()) {
                    qWarning() << "SSSS: Failed to calculate HMAC";
                    // emit error(DecryptionError);
                    return;
                }

                auto mac = result.value();

                auto identifier = QString::fromLatin1(QCryptographicHash::hash(QUuid::createUuid().toString().toLatin1(), QCryptographicHash::Sha256));

                setAccountData(QStringLiteral("m.secret_storage.key.%1").arg(identifier),
                               {
                                   {"algorithm"_ls, "m.secret_storage.v1.aes-hmac-sha2"_ls},
                                   {"iv"_ls, QString::fromLatin1(iv.toBase64())},
                                   {"mac"_ls, QString::fromLatin1(mac.toBase64())},
                               });
                setAccountData(QStringLiteral("m.secret_storage.default_key"),
                               {
                                   {"key"_ls, identifier},
                               });

                // TODO make sure masterKeyForUser already works at this point;
                database()->setMasterKeyVerified(masterKeyForUser(userId()));

                // TODO store keys in accountdata
                // TODO start a key backup and store in account data

                qWarning() << "finished uploading cs keys" << job->jsonData() << job->errorString();
            });
    });
}

#include "moc_neochatconnection.cpp"
