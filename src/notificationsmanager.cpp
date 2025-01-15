// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "notificationsmanager.h"

#include <memory>

#include <QGuiApplication>

#include <KLocalizedString>
#include <KNotification>
#include <KNotificationPermission>
#include <KNotificationReplyAction>

#include <QPainter>
#include <Quotient/accountregistry.h>
#include <Quotient/csapi/pushrules.h>
#include <Quotient/events/roommemberevent.h>
#include <Quotient/user.h>

#ifdef HAVE_KIO
#include <KIO/ApplicationLauncherJob>
#endif

#include "controller.h"
#include "jobs/neochatgetcommonroomsjob.h"
#include "neochatconfig.h"
#include "neochatconnection.h"
#include "neochatroom.h"
#include "roommanager.h"
#include "texthandler.h"
#include "windowcontroller.h"

using namespace Quotient;

NotificationsManager::NotificationsManager(QObject *parent)
    : QObject(parent)
{
}

void NotificationsManager::handleNotifications(QPointer<NeoChatConnection> connection)
{
    if (KNotificationPermission::checkPermission() == Qt::PermissionStatus::Granted) {
        startNotificationJob(connection);
    } else if (!permissionAsked) {
        KNotificationPermission::requestPermission(this, [this, connection](Qt::PermissionStatus result) {
            if (result == Qt::PermissionStatus::Granted) {
                startNotificationJob(connection);
            } else {
                permissionAsked = true;
            }
        });
    }
}

void NotificationsManager::startNotificationJob(QPointer<NeoChatConnection> connection)
{
    if (connection == nullptr) {
        return;
    }

    if (!m_connActiveJob.contains(connection->user()->id())) {
        auto job = connection->callApi<GetNotificationsJob>();
        m_connActiveJob.append(connection->user()->id());
        connect(job, &BaseJob::success, this, [this, job, connection]() {
            m_connActiveJob.removeAll(connection->user()->id());
            processNotificationJob(connection, job, !m_oldNotifications.contains(connection->user()->id()));
        });
    }
}

void NotificationsManager::processNotificationJob(QPointer<NeoChatConnection> connection, Quotient::GetNotificationsJob *job, bool initialization)
{
    if (!job || !connection || !connection->isLoggedIn()) {
        return;
    }

    const auto connectionId = connection->user()->id();

    const auto notifications = job->jsonData()["notifications"_L1].toArray();
    if (initialization) {
        for (const auto &notification : notifications) {
            if (!m_initialTimestamp.contains(connectionId)) {
                m_initialTimestamp[connectionId] = notification["ts"_L1].toVariant().toLongLong();
            } else {
                qint64 timestamp = notification["ts"_L1].toVariant().toLongLong();
                if (timestamp > m_initialTimestamp[connectionId]) {
                    m_initialTimestamp[connectionId] = timestamp;
                }
            }

            auto connectionNotifications = m_oldNotifications.value(connectionId);
            connectionNotifications += notification["event"_L1]["event_id"_L1].toString();
            m_oldNotifications[connectionId] = connectionNotifications;
        }
        return;
    }

    QMap<QString, std::pair<qint64, QJsonObject>> notificationsToPost;
    for (const auto &n : notifications) {
        const auto notification = n.toObject();
        if (notification["read"_L1].toBool()) {
            continue;
        }
        auto connectionNotifications = m_oldNotifications.value(connectionId);
        if (connectionNotifications.contains(notification["event"_L1]["event_id"_L1].toString())) {
            continue;
        }
        connectionNotifications += notification["event"_L1]["event_id"_L1].toString();
        m_oldNotifications[connectionId] = connectionNotifications;

        if (!shouldPostNotification(connection, n)) {
            continue;
        }

        const auto &roomId = notification["room_id"_L1].toString();
        if (!notificationsToPost.contains(roomId) || notificationsToPost[roomId].first < notification["ts"_L1].toVariant().toLongLong()) {
            notificationsToPost[roomId] = {notification["ts"_L1].toVariant().toLongLong(), notification};
        }
    }

    for (const auto &[roomId, pair] : notificationsToPost.asKeyValueRange()) {
        const auto &notification = pair.second;
        const auto room = connection->room(roomId);
        if (!room) {
            continue;
        }
        auto sender = room->member(notification["event"_L1]["sender"_L1].toString());

        if (room->joinState() == JoinState::Invite) {
            postInviteNotification(qobject_cast<NeoChatRoom *>(room));
            continue;
        }

        QString body;
        if (notification["event"_L1]["type"_L1].toString() == "org.matrix.msc3381.poll.start"_L1) {
            body = notification["event"_L1]["content"_L1]["org.matrix.msc3381.poll.start"_L1]["question"_L1]["body"_L1].toString();
        } else if (notification["event"_L1]["type"_L1] == "m.room.encrypted"_L1) {
            const auto decrypted = connection->decryptNotification(notification);
            body = decrypted["content"_L1]["body"_L1].toString();
            if (body.isEmpty()) {
                body = i18n("Encrypted Message");
            }
        } else {
            body = notification["event"_L1]["content"_L1]["body"_L1].toString();
        }

        QImage avatar_image;
        if (!sender.avatarUrl().isEmpty()) {
            avatar_image = room->member(sender.id()).avatar(128, 128, {});
        } else {
            avatar_image = room->avatar(128);
        }
        postNotification(dynamic_cast<NeoChatRoom *>(room),
                         sender.displayName(),
                         body,
                         avatar_image,
                         notification["event"_L1].toObject()["event_id"_L1].toString(),
                         true,
                         pair.first);
    }
}

bool NotificationsManager::shouldPostNotification(QPointer<NeoChatConnection> connection, const QJsonValue &notification)
{
    if (connection == nullptr || !connection->isLoggedIn()) {
        return false;
    }

    auto room = connection->room(notification["room_id"_L1].toString());
    if (room == nullptr) {
        return false;
    }

    // If the room is the current room and the application is active the notification
    // should not be shown.
    // This is setup so that if the application is inactive the notification will
    // always be posted, even if the room is the current room.
    bool isCurrentRoom = RoomManager::instance().currentRoom() && room->id() == RoomManager::instance().currentRoom()->id();
    if (isCurrentRoom && QGuiApplication::applicationState() == Qt::ApplicationActive) {
        return false;
    }

    // If the notification timestamp is earlier than the initial timestamp assume
    // the notification is old and shouldn't be posted.
    qint64 timestamp = notification["ts"_L1].toDouble();
    if (timestamp < m_initialTimestamp[connection->user()->id()]) {
        return false;
    }

    if (m_notifications.contains(room->id()) && m_notifications[room->id()].first > timestamp) {
        return false;
    }

    return true;
}

void NotificationsManager::postNotification(NeoChatRoom *room,
                                            const QString &sender,
                                            const QString &text,
                                            const QImage &icon,
                                            const QString &replyEventId,
                                            bool canReply,
                                            qint64 timestamp)
{
    const QString roomId = room->id();

    if (auto notification = m_notifications.value(roomId).second) {
        notification->close();
    }

    auto notification = new KNotification(u"message"_s);
    m_notifications.insert(roomId, {timestamp, notification});
    connect(notification, &KNotification::closed, this, [this, roomId, notification] {
        if (m_notifications[roomId].second == notification) {
            m_notifications.remove(roomId);
        }
    });

    QString entry;
    if (sender == room->displayName()) {
        notification->setTitle(sender);
        entry = text.toHtmlEscaped();
    } else {
        notification->setTitle(room->displayName());
        entry = i18n("%1: %2", sender, text.toHtmlEscaped());
    }

    notification->setText(entry);
    notification->setPixmap(createNotificationImage(icon, room));

    auto defaultAction = notification->addDefaultAction(i18n("Open NeoChat in this room"));
    connect(defaultAction, &KNotificationAction::activated, this, [notification, room]() {
        WindowController::instance().showAndRaiseWindow(notification->xdgActivationToken());
        if (!room) {
            return;
        }
        auto connection = dynamic_cast<NeoChatConnection *>(Controller::instance().accounts().get(room->localMember().id()));
        Controller::instance().setActiveConnection(connection);
        RoomManager::instance().setConnection(connection);
        RoomManager::instance().resolveResource(room->id());
    });

    if (canReply) {
        std::unique_ptr<KNotificationReplyAction> replyAction(new KNotificationReplyAction(i18n("Reply")));
        replyAction->setPlaceholderText(i18n("Reply..."));
        connect(replyAction.get(), &KNotificationReplyAction::replied, this, [room, replyEventId](const QString &text) {
            TextHandler textHandler;
            textHandler.setData(text);
            auto content = std::make_unique<Quotient::EventContent::TextContent>(textHandler.handleSendText(), u"text/html"_s);
            EventRelation relatesTo = EventRelation::replyTo(replyEventId);
            room->post<Quotient::RoomMessageEvent>(text, MessageEventType::Text, std::move(content), relatesTo);
        });
        notification->setReplyAction(std::move(replyAction));
    }

    notification->setHint(u"x-kde-origin-name"_s, room->localMember().id());
    notification->sendEvent();
}

void NotificationsManager::postInviteNotification(NeoChatRoom *rawRoom)
{
    QPointer room(rawRoom);

    const auto roomMemberEvent = room->currentState().get<RoomMemberEvent>(room->localMember().id());
    if (roomMemberEvent == nullptr) {
        return;
    }

    if (NeoChatConfig::rejectUnknownInvites()) {
        auto job = room->connection()->callApi<NeochatGetCommonRoomsJob>(roomMemberEvent->senderId());
        connect(job, &BaseJob::result, this, [this, job, room] {
            QJsonObject replyData = job->jsonData();
            if (replyData.contains(u"joined"_s)) {
                const bool inAnyOfOurRooms = !replyData["joined"_L1].toArray().isEmpty();
                if (inAnyOfOurRooms) {
                    doPostInviteNotification(room);
                } else {
                    room->leaveRoom();
                }
            }
        });
    } else {
        doPostInviteNotification(room);
    }
}

void NotificationsManager::doPostInviteNotification(QPointer<NeoChatRoom> room)
{
    const auto roomMemberEvent = room->currentState().get<RoomMemberEvent>(room->localMember().id());
    if (roomMemberEvent == nullptr) {
        return;
    }
    const auto sender = room->member(roomMemberEvent->senderId());

    QImage avatar_image;
    if (roomMemberEvent && !room->member(roomMemberEvent->senderId()).avatarUrl().isEmpty()) {
        avatar_image = room->member(roomMemberEvent->senderId()).avatar(128, 128, {});
    } else {
        qWarning() << "using this room's avatar";
        avatar_image = room->avatar(128);
    }

    KNotification *notification = new KNotification(u"invite"_s);
    notification->setText(i18n("%1 invited you to a room", sender.htmlSafeDisplayName()));
    notification->setTitle(room->displayName());
    notification->setPixmap(createNotificationImage(avatar_image, nullptr));
    auto defaultAction = notification->addDefaultAction(i18n("Open this invitation in NeoChat"));
    connect(defaultAction, &KNotificationAction::activated, this, [notification, room]() {
        if (!room) {
            return;
        }
        WindowController::instance().showAndRaiseWindow(notification->xdgActivationToken());
        notification->close();
        RoomManager::instance().resolveResource(room->id());
    });

    const auto acceptAction = notification->addAction(i18nc("@action:button The thing being accepted is an invitation to chat", "Accept"));
    const auto rejectAction = notification->addAction(i18nc("@action:button The thing being rejected is an invitation to chat", "Reject"));
    const auto rejectAndIgnoreAction = notification->addAction(i18nc("@action:button The thing being rejected is an invitation to chat", "Reject and Ignore User"));
    connect(acceptAction, &KNotificationAction::activated, this, [room, notification]() {
        if (!room) {
            return;
        }
        room->acceptInvitation();
        notification->close();
    });
    connect(rejectAction, &KNotificationAction::activated, this, [room, notification]() {
        if (!room) {
            return;
        }
        RoomManager::instance().leaveRoom(room);
        notification->close();
    });
    connect(rejectAndIgnoreAction, &KNotificationAction::activated, this, [room, notification]() {
        if (!room) {
            return;
        }
        RoomManager::instance().leaveRoom(room);
        room->connection()->addToIgnoredUsers(room->invitingUserId());
        notification->close();
    });
    connect(notification, &KNotification::closed, this, [this, room]() {
        if (!room) {
            return;
        }
        m_invitations.remove(room->id());
    });

    notification->setHint(u"x-kde-origin-name"_s, room->localMember().id());

    notification->sendEvent();
}

void NotificationsManager::clearInvitationNotification(const QString &roomId)
{
    if (m_invitations.contains(roomId)) {
        m_invitations[roomId]->close();
    }
}

void NotificationsManager::postPushNotification(const QByteArray &message)
{
    const auto json = QJsonDocument::fromJson(message).object();

    const auto type = json["notification"_L1]["type"_L1].toString();

    // the only two types of push notifications we support right now
    if (type == u"m.room.message"_s || type == u"m.room.encrypted"_s) {
        auto notification = new KNotification("message"_L1);

        const auto sender = json["notification"_L1]["sender_display_name"_L1].toString();
        const auto roomName = json["notification"_L1]["room_name"_L1].toString();
        const auto roomId = json["notification"_L1]["room_id"_L1].toString();

        if (roomName.isEmpty() || sender == roomName) {
            notification->setTitle(sender);
        } else {
            notification->setTitle(i18n("%1 (%2)", sender, roomName));
        }

        if (type == u"m.room.message"_s) {
            const auto text = json["notification"_L1]["content"_L1]["body"_L1].toString();
            notification->setText(text.toHtmlEscaped());
        } else if (type == u"m.room.encrypted"_s) {
            notification->setText(i18n("Encrypted Message"));
        }

#ifdef HAVE_KIO
        auto openAction = notification->addAction(i18n("Open NeoChat"));
        connect(openAction, &KNotificationAction::activated, this, [=]() {
            QString properId = roomId;
            properId = properId.replace(u"#"_s, QString());
            properId = properId.replace(u"!"_s, QString());

            auto *job = new KIO::ApplicationLauncherJob(KService::serviceByDesktopName(u"org.kde.neochat"_s));
            job->setUrls({QUrl::fromUserInput(u"matrix:r/%1"_s.arg(properId))});
            job->start();
        });
#endif

        connect(notification, &KNotification::closed, qGuiApp, &QGuiApplication::quit);

        notification->sendEvent();

        m_notifications.insert(roomId, {json["ts"_L1].toVariant().toLongLong(), notification});
    } else {
        qWarning() << "Skipping unsupported push notification" << type;
    }
}

QPixmap NotificationsManager::createNotificationImage(const QImage &icon, NeoChatRoom *room)
{
    // Handle avatars that are lopsided in one dimension
    const int biggestDimension = std::max(icon.width(), icon.height());
    const QRect imageRect{0, 0, biggestDimension, biggestDimension};

    QImage roundedImage(imageRect.size(), QImage::Format_ARGB32);
    roundedImage.fill(Qt::transparent);

    QPainter painter(&roundedImage);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);

    // Fill background for transparent avatars
    painter.setBrush(Qt::white);
    painter.drawRoundedRect(imageRect, imageRect.width(), imageRect.height());

    QBrush brush(icon.scaledToHeight(biggestDimension));
    painter.setBrush(brush);
    painter.drawRoundedRect(imageRect, imageRect.width(), imageRect.height());

    if (room != nullptr) {
        const QImage roomAvatar = room->avatar(imageRect.width(), imageRect.height());
        if (icon != roomAvatar) {
            const QRect lowerQuarter{imageRect.center(), imageRect.size() / 2};

            painter.setBrush(Qt::white);
            painter.drawRoundedRect(lowerQuarter, lowerQuarter.width(), lowerQuarter.height());

            painter.setBrush(roomAvatar.scaled(lowerQuarter.size()));
            painter.drawRoundedRect(lowerQuarter, lowerQuarter.width(), lowerQuarter.height());
        }
    }

    return QPixmap::fromImage(roundedImage);
}

#include "moc_notificationsmanager.cpp"
