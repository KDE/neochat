// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatconnection.h"

#include <QImageReader>
#include <QJsonDocument>

#include "neochatroom.h"
#include "spacehierarchycache.h"

#include <Quotient/connection.h>
#include <Quotient/jobs/basejob.h>
#include <Quotient/quotient_common.h>
#include <qt6keychain/keychain.h>

#include <KLocalizedString>

#include <Quotient/csapi/content-repo.h>
#include <Quotient/csapi/profile.h>
#include <Quotient/csapi/registration.h>
#include <Quotient/csapi/versions.h>
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

bool NeoChatConnection::m_globalUrlPreviewDefault = true;
PushRuleAction::Action NeoChatConnection::m_defaultAction = PushRuleAction::Unknown;

NeoChatConnection::NeoChatConnection(QObject *parent)
    : Connection(parent)
{
    m_linkPreviewers.setMaxCost(20);
    connectSignals();
}

NeoChatConnection::NeoChatConnection(const QUrl &server, QObject *parent)
    : Connection(server, parent)
{
    m_linkPreviewers.setMaxCost(20);
    connectSignals();
}

void NeoChatConnection::connectSignals()
{
    connect(this, &NeoChatConnection::accountDataChanged, this, [this](const QString &type) {
        if (type == u"org.kde.neochat.account_label"_s) {
            Q_EMIT labelChanged();
        }
        if (type == u"m.identity_server"_s) {
            Q_EMIT identityServerChanged();
        }
    });
    connect(this, &NeoChatConnection::requestFailed, this, [this](BaseJob *job) {
        if (job->error() == BaseJob::UserConsentRequired) {
            Q_EMIT userConsentRequired(job->errorUrl());
        }
    });
    connect(this, &NeoChatConnection::requestFailed, this, [this](BaseJob *job) {
        if (dynamic_cast<DownloadFileJob *>(job) && job->jsonData()["errcode"_L1].toString() == "M_TOO_LARGE"_L1) {
            Q_EMIT showMessage(MessageType::Warning, i18n("File too large to download.<br />Contact your matrix server administrator for support."));
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

    connect(this, &NeoChatConnection::globalUrlPreviewEnabledChanged, this, [this]() {
        if (!m_globalUrlPreviewDefault) {
            m_linkPreviewers.clear();
        }
    });

    // Fetch unstable features
    // TODO: Expose unstableFeatures() in libQuotient
    connect(
        this,
        &Connection::connected,
        this,
        [this] {
            callApi<GetVersionsJob>(BackgroundRequest).onResult([this](const auto &job) {
                m_canCheckMutualRooms = job->unstableFeatures().contains("uk.half-shot.msc2666.query_mutual_rooms"_L1);
                Q_EMIT canCheckMutualRoomsChanged();
                m_canEraseData = job->unstableFeatures().contains("org.matrix.msc4025"_L1) || job->versions().count("v1.10"_L1);
                Q_EMIT canEraseDataChanged();
            });
        },
        Qt::SingleShotConnection);

    connect(this, &Connection::sessionVerified, this, [this](const QString &userId, const QString &deviceId) {
        if (userId == this->userId() && deviceId == this->deviceId()) {
            Q_EMIT ownSessionVerified();
        }
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
        Q_EMIT badgeNotificationCountChanged(m_badgeNotificationCount);
    }
}

bool NeoChatConnection::globalUrlPreviewEnabled()
{
    return m_globalUrlPreviewDefault;
}

void NeoChatConnection::setGlobalUrlPreviewDefault(bool useByDefault)
{
    NeoChatConnection::m_globalUrlPreviewDefault = useByDefault;
}

PushRuleAction::Action NeoChatConnection::keywordPushRuleDefault() const
{
    return m_defaultAction;
}

void NeoChatConnection::setKeywordPushRuleDefault(PushRuleAction::Action defaultAction)
{
    NeoChatConnection::m_defaultAction = defaultAction;
}

void NeoChatConnection::logout(bool serverSideLogout)
{
    SettingsGroup(u"Accounts"_s).remove(userId());

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
        roomVersionMap.insert("id"_L1, v.id);
        roomVersionMap.insert("status"_L1, v.status);
        roomVersionMap.insert("isStable"_L1, v.isStable());
        supportedRoomVersions.append(roomVersionMap);
    }
    return supportedRoomVersions;
}

bool NeoChatConnection::canCheckMutualRooms() const
{
    return m_canCheckMutualRooms;
}

void NeoChatConnection::changePassword(const QString &currentPassword, const QString &newPassword)
{
    callApi<ChangePasswordJob>(newPassword, false).onFailure([this, currentPassword, newPassword](const auto &job) {
        QJsonObject replyData = job->jsonData();
        AuthenticationData authData;
        authData.session = replyData["session"_L1].toString();
        authData.type = "m.login.password"_L1;
        authData.authInfo["password"_L1] = currentPassword;
        authData.authInfo["user"_L1] = user()->id();
        authData.authInfo["identifier"_L1] = QJsonObject{{"type"_L1, "m.id.user"_L1}, {"user"_L1, user()->id()}};
        auto innerJob = callApi<ChangePasswordJob>(newPassword, false, authData)
                            .then(
                                [this]() {
                                    Q_EMIT passwordStatus(PasswordStatus::Success);
                                },
                                [this](const auto &job) {
                                    Q_EMIT passwordStatus(job->jsonData()["errcode"_L1] == "M_FORBIDDEN"_L1 ? PasswordStatus::Wrong : PasswordStatus::Other);
                                });
    });
}

void NeoChatConnection::setLabel(const QString &label)
{
    QJsonObject json{
        {"account_label"_L1, label},
    };
    setAccountData("org.kde.neochat.account_label"_L1, json);
    Q_EMIT labelChanged();
}

QString NeoChatConnection::label() const
{
    return accountDataJson("org.kde.neochat.account_label"_L1)["account_label"_L1].toString();
}

void NeoChatConnection::deactivateAccount(const QString &password, const bool erase)
{
    callApi<DeactivateAccountJob>().onFailure([password, erase, this](const auto &job) {
        QJsonObject replyData = job->jsonData();
        AuthenticationData authData;
        authData.session = replyData["session"_L1].toString();
        authData.authInfo["password"_L1] = password;
        authData.type = "m.login.password"_L1;
        authData.authInfo["user"_L1] = user()->id();
        QJsonObject identifier = {{"type"_L1, "m.id.user"_L1}, {"user"_L1, user()->id()}};
        authData.authInfo["identifier"_L1] = identifier;
        callApi<DeactivateAccountJob>(authData, QString{}, erase).onResult([this]() {
            logout(false);
        });
    });
}

bool NeoChatConnection::hasIdentityServer() const
{
    if (!hasAccountData(u"m.identity_server"_s)) {
        return false;
    }

    const auto url = accountData(u"m.identity_server"_s)->contentPart<QUrl>("base_url"_L1);
    if (!url.isEmpty()) {
        return true;
    }
    return false;
}

QUrl NeoChatConnection::identityServer() const
{
    if (!hasAccountData(u"m.identity_server"_s)) {
        return {};
    }

    const auto url = accountData(u"m.identity_server"_s)->contentPart<QUrl>("base_url"_L1);
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
            "m.space.parent"_L1,
            QJsonObject{
                {"canonical"_L1, true},
                {"via"_L1, QJsonArray{domain()}},
            },
            parent,
        });
    }

    Connection::createRoom(Connection::PublishRoom, QString(), name, topic, QStringList(), {}, {}, {}, initialStateEvents)
        .then(
            [parent, setChildParent, this](const auto &job) {
                if (parent.isEmpty() || !setChildParent) {
                    return;
                }
                if (auto parentRoom = room(parent)) {
                    parentRoom->setState(u"m.space.child"_s, job->roomId(), QJsonObject{{"via"_L1, QJsonArray{domain()}}});
                }
            },
            [this](const auto &job) {
                Q_EMIT errorOccured(i18n("Room creation failed: %1", job->errorString()));
            });
}

void NeoChatConnection::createSpace(const QString &name, const QString &topic, const QString &parent, bool setChildParent)
{
    QList<CreateRoomJob::StateEvent> initialStateEvents;
    if (!parent.isEmpty()) {
        initialStateEvents.append(CreateRoomJob::StateEvent{
            "m.space.parent"_L1,
            QJsonObject{
                {"canonical"_L1, true},
                {"via"_L1, QJsonArray{domain()}},
            },
            parent,
        });
    }

    Connection::createRoom(Connection::UnpublishRoom, {}, name, topic, {}, {}, {}, false, initialStateEvents, {}, QJsonObject{{"type"_L1, "m.space"_L1}})
        .then(
            [parent, setChildParent, this](const auto &job) {
                if (parent.isEmpty() || !setChildParent) {
                    return;
                }
                if (auto parentRoom = room(parent)) {
                    parentRoom->setState(u"m.space.child"_s, job->roomId(), QJsonObject{{"via"_L1, QJsonArray{domain()}}});
                }
            },
            [this](const auto &job) {
                Q_EMIT errorOccured(i18n("Space creation failed: %1", job->errorString()));
            });
}

Quotient::ForgetRoomJob *NeoChatConnection::forgetRoom(const QString &id)
{
    Q_EMIT roomAboutToBeLeft(id);

    return Connection::forgetRoom(id);
}

bool NeoChatConnection::directChatExists(Quotient::User *user)
{
    return directChats().contains(user);
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
    gatewayEndpoint.setPath(u"/_matrix/push/v1/notify"_s);

    QNetworkRequest checkGateway(gatewayEndpoint);
    auto reply = co_await NetworkAccessManager::instance()->get(checkGateway);

    // We want to check if this UnifiedPush server has a Matrix gateway
    // This is because Matrix does not natively support UnifiedPush
    const auto &replyJson = QJsonDocument::fromJson(reply->readAll()).object();

    if (replyJson["unifiedpush"_L1]["gateway"_L1].toString() == u"matrix"_s) {
        callApi<PostPusherJob>(endpoint,
                               u"http"_s,
                               u"org.kde.neochat"_s,
                               u"NeoChat"_s,
                               deviceId(),
                               QString(), // profileTag is intentionally left empty for now, it's optional
                               u"en-US"_s,
                               PostPusherJob::PusherData{QUrl::fromUserInput(gatewayEndpoint.toString()), u" "_s},
                               false);

        qInfo() << "Registered for push notifications";
        m_pushNotificationsEnabled = true;
    } else {
        qWarning() << "There's no gateway, not setting up push notifications.";
        m_pushNotificationsEnabled = false;
    }
    Q_EMIT enablePushNotificationsChanged();
#else
    Q_UNUSED(endpoint)
    co_return;
#endif
}

QString NeoChatConnection::accountDataJsonString(const QString &type) const
{
    return QString::fromUtf8(QJsonDocument(accountDataJson(type)).toJson());
}

LinkPreviewer *NeoChatConnection::previewerForLink(const QUrl &link)
{
    if (!m_globalUrlPreviewDefault) {
        return nullptr;
    }

    auto previewer = m_linkPreviewers.object(link);
    if (previewer != nullptr) {
        return previewer;
    }

    previewer = new LinkPreviewer(link, this);
    m_linkPreviewers.insert(link, previewer);
    return previewer;
}

KeyImport::Error NeoChatConnection::exportMegolmSessions(const QString &passphrase, const QString &path)
{
    KeyImport keyImport;
    auto result = keyImport.exportKeys(passphrase, this);
    if (!result.has_value()) {
        return result.error();
    }
    QUrl url(path);
    QFile file(url.toLocalFile());
    auto ok = file.open(QFile::WriteOnly);
    if (!ok) {
        qWarning() << "Failed to open" << file.fileName() << file.errorString();
        return KeyImport::OtherError;
    }
    file.write(result.value());
    file.close();
    return KeyImport::Success;
}

bool NeoChatConnection::canEraseData() const
{
    return m_canEraseData;
}

bool NeoChatConnection::pushNotificationsAvailable() const
{
#ifdef HAVE_KUNIFIEDPUSH
    return true;
#else
    return false;
#endif
}

bool NeoChatConnection::enablePushNotifications() const
{
    return m_pushNotificationsEnabled;
}

bool NeoChatConnection::isVerifiedSession() const
{
    return isVerifiedDevice(userId(), deviceId());
}

#include "moc_neochatconnection.cpp"
