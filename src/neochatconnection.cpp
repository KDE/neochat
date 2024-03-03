// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatconnection.h"

#include <QImageReader>
#include <QJsonDocument>

#include "controller.h"
#include "jobs/neochatchangepasswordjob.h"
#include "jobs/neochatdeactivateaccountjob.h"
#include "neochatroom.h"
#include "roommanager.h"
#include "spacehierarchycache.h"

#include <Quotient/connection.h>
#include <Quotient/quotient_common.h>
#include <qt6keychain/keychain.h>

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
{
    connectSignals();
}

NeoChatConnection::NeoChatConnection(const QUrl &server, QObject *parent)
    : Connection(server, parent)
{
    connectSignals();
}

void NeoChatConnection::connectSignals()
{
    connect(this, &NeoChatConnection::accountDataChanged, this, [this](const QString &type) {
        if (type == QLatin1String("org.kde.neochat.account_label")) {
            Q_EMIT labelChanged();
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

    if (Controller::instance().accounts().count() > 1) {
        // Only set the connection if the the account being logged out is currently active
        if (this == Controller::instance().activeConnection()) {
            Controller::instance().setActiveConnection(dynamic_cast<NeoChatConnection *>(Controller::instance().accounts().accounts()[0]));
        }
    } else {
        Controller::instance().setActiveConnection(nullptr);
    }
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
    requestDirectChat(user);
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

void NeoChatConnection::addRoom(Quotient::Room *room)
{
    Connection::addRoom(room, false);
}

#include "moc_neochatconnection.cpp"
