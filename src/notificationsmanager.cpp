// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "notificationsmanager.h"

#include <memory>

#include <QGuiApplication>

#include <KLocalizedString>
#include <KNotification>
#include <KNotificationReplyAction>

#include <QPainter>
#include <Quotient/accountregistry.h>
#include <Quotient/csapi/pushrules.h>
#include <Quotient/user.h>

#ifdef HAVE_KIO
#include <KIO/ApplicationLauncherJob>
#endif

#include "controller.h"
#include "neochatconnection.h"
#include "neochatroom.h"
#include "roommanager.h"
#include "texthandler.h"
#include "windowcontroller.h"

using namespace Quotient;

NotificationsManager &NotificationsManager::instance()
{
    static NotificationsManager _instance;
    return _instance;
}

NotificationsManager::NotificationsManager(QObject *parent)
    : QObject(parent)
{
}

void NotificationsManager::handleNotifications(QPointer<NeoChatConnection> connection)
{
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

    const auto notifications = job->jsonData()["notifications"_ls].toArray();
    if (initialization) {
        for (const auto &notification : notifications) {
            if (!m_initialTimestamp.contains(connectionId)) {
                m_initialTimestamp[connectionId] = notification["ts"_ls].toVariant().toLongLong();
            } else {
                qint64 timestamp = notification["ts"_ls].toVariant().toLongLong();
                if (timestamp > m_initialTimestamp[connectionId]) {
                    m_initialTimestamp[connectionId] = timestamp;
                }
            }

            auto connectionNotifications = m_oldNotifications.value(connectionId);
            connectionNotifications += notification["event"_ls]["event_id"_ls].toString();
            m_oldNotifications[connectionId] = connectionNotifications;
        }
        return;
    }

    QMap<QString, std::pair<qint64, QJsonObject>> notificationsToPost;
    for (const auto &n : notifications) {
        const auto notification = n.toObject();
        if (notification["read"_ls].toBool()) {
            continue;
        }
        auto connectionNotifications = m_oldNotifications.value(connectionId);
        if (connectionNotifications.contains(notification["event"_ls]["event_id"_ls].toString())) {
            continue;
        }
        connectionNotifications += notification["event"_ls]["event_id"_ls].toString();
        m_oldNotifications[connectionId] = connectionNotifications;

        if (!shouldPostNotification(connection, n)) {
            continue;
        }

        const auto &roomId = notification["room_id"_ls].toString();
        if (!notificationsToPost.contains(roomId) || notificationsToPost[roomId].first < notification["ts"_ls].toVariant().toLongLong()) {
            notificationsToPost[roomId] = {notification["ts"_ls].toVariant().toLongLong(), notification};
        }
    }

    for (const auto &[roomId, pair] : notificationsToPost.asKeyValueRange()) {
        const auto &notification = pair.second;
        const auto room = connection->room(roomId);
        if (!room) {
            continue;
        }
        auto sender = room->user(notification["event"_ls]["sender"_ls].toString());

        QString body;
        if (notification["event"_ls]["type"_ls].toString() == "org.matrix.msc3381.poll.start"_ls) {
            body = notification["event"_ls]["content"_ls]["org.matrix.msc3381.poll.start"_ls]["question"_ls]["body"_ls].toString();
        } else if (notification["event"_ls]["type"_ls] == "m.room.encrypted"_ls) {
            const auto decrypted = connection->decryptNotification(notification);
            body = decrypted["content"_ls]["body"_ls].toString();
            if (body.isEmpty()) {
                body = i18n("Encrypted Message");
            }
        } else {
            body = notification["event"_ls]["content"_ls]["body"_ls].toString();
        }

        QImage avatar_image;
        if (!sender->avatarUrl(room).isEmpty()) {
            avatar_image = sender->avatar(128, room);
        } else {
            avatar_image = room->avatar(128);
        }
        postNotification(dynamic_cast<NeoChatRoom *>(room),
                         sender->displayname(room),
                         body,
                         avatar_image,
                         notification["event"_ls].toObject()["event_id"_ls].toString(),
                         true,
                         pair.first);
    }
}

bool NotificationsManager::shouldPostNotification(QPointer<NeoChatConnection> connection, const QJsonValue &notification)
{
    if (connection == nullptr || !connection->isLoggedIn()) {
        return false;
    }

    auto room = connection->room(notification["room_id"_ls].toString());
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
    qint64 timestamp = notification["ts"_ls].toDouble();
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

    auto notification = new KNotification(QStringLiteral("message"));
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
        auto connection = dynamic_cast<NeoChatConnection *>(Controller::instance().accounts().get(room->localUser()->id()));
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
            room->postMessage(text, textHandler.handleSendText(), RoomMessageEvent::MsgType::Text, replyEventId, QString());
        });
        notification->setReplyAction(std::move(replyAction));
    }

    notification->setHint(QStringLiteral("x-kde-origin-name"), room->localUser()->id());
    notification->sendEvent();
}

void NotificationsManager::postInviteNotification(NeoChatRoom *rawRoom, const QString &title, const QString &sender, const QImage &icon)
{
    QPointer room(rawRoom);
    QPixmap img;
    img.convertFromImage(icon);
    KNotification *notification = new KNotification(QStringLiteral("invite"));
    notification->setText(i18n("%1 invited you to a room", sender));
    notification->setTitle(title);
    notification->setPixmap(createNotificationImage(icon, nullptr));
    notification->setFlags(KNotification::Persistent);
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
        room->connection()->addToIgnoredUsers(room->invitingUser());
        notification->close();
    });
    connect(notification, &KNotification::closed, this, [this, room]() {
        if (!room) {
            return;
        }
        m_invitations.remove(room->id());
    });

    notification->setHint(QStringLiteral("x-kde-origin-name"), room->localUser()->id());

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

    const auto type = json["notification"_ls]["type"_ls].toString();

    // the only two types of push notifications we support right now
    if (type == QStringLiteral("m.room.message") || type == QStringLiteral("m.room.encrypted")) {
        auto notification = new KNotification("message"_ls);

        const auto sender = json["notification"_ls]["sender_display_name"_ls].toString();
        const auto roomName = json["notification"_ls]["room_name"_ls].toString();
        const auto roomId = json["notification"_ls]["room_id"_ls].toString();

        if (roomName.isEmpty() || sender == roomName) {
            notification->setTitle(sender);
        } else {
            notification->setTitle(i18n("%1 (%2)", sender, roomName));
        }

        if (type == QStringLiteral("m.room.message")) {
            const auto text = json["notification"_ls]["content"_ls]["body"_ls].toString();
            notification->setText(text.toHtmlEscaped());
        } else if (type == QStringLiteral("m.room.encrypted")) {
            notification->setText(i18n("Encrypted Message"));
        }

#ifdef HAVE_KIO
        auto openAction = notification->addAction(i18n("Open NeoChat"));
        connect(openAction, &KNotificationAction::activated, this, [=]() {
            QString properId = roomId;
            properId = properId.replace(QStringLiteral("#"), QString());
            properId = properId.replace(QStringLiteral("!"), QString());

            auto *job = new KIO::ApplicationLauncherJob(KService::serviceByDesktopName(QStringLiteral("org.kde.neochat")));
            job->setUrls({QUrl::fromUserInput(QStringLiteral("matrix:r/%1").arg(properId))});
            job->start();
        });
#endif

        connect(notification, &KNotification::closed, qGuiApp, &QGuiApplication::quit);

        notification->sendEvent();

        m_notifications.insert(roomId, {json["ts"_ls].toVariant().toLongLong(), notification});
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
