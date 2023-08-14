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
#include <Quotient/connection.h>
#include <Quotient/csapi/pushrules.h>
#include <Quotient/user.h>

#include "controller.h"
#include "neochatconfig.h"
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

void NotificationsManager::handleNotifications(QPointer<Connection> connection)
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

void NotificationsManager::processNotificationJob(QPointer<Quotient::Connection> connection, Quotient::GetNotificationsJob *job, bool initialization)
{
    if (job == nullptr) {
        return;
    }
    if (connection == nullptr) {
        qWarning() << QStringLiteral("No connection for GetNotificationsJob %1").arg(job->objectName());
        return;
    }

    const auto connectionId = connection->user()->id();

    // If pagination has occurred set off the next job
    auto nextToken = job->jsonData()["next_token"].toString();
    if (!nextToken.isEmpty()) {
        auto nextJob = connection->callApi<GetNotificationsJob>(nextToken);
        m_connActiveJob.append(connectionId);
        connect(nextJob, &BaseJob::success, this, [this, nextJob, connection, initialization]() {
            m_connActiveJob.removeAll(connection->user()->id());
            processNotificationJob(connection, nextJob, initialization);
        });
    }

    const auto notifications = job->jsonData()["notifications"].toArray();
    if (initialization) {
        m_oldNotifications[connectionId] = QStringList();
        for (const auto &n : notifications) {
            if (!m_initialTimestamp.contains(connectionId)) {
                m_initialTimestamp[connectionId] = n.toObject()["ts"].toDouble();
            } else {
                qint64 timestamp = n.toObject()["ts"].toDouble();
                if (timestamp > m_initialTimestamp[connectionId]) {
                    m_initialTimestamp[connectionId] = timestamp;
                }
            }

            auto connectionNotifications = m_oldNotifications.value(connectionId);
            connectionNotifications += n.toObject()["event"].toObject()["event_id"].toString();
            m_oldNotifications[connectionId] = connectionNotifications;
        }
        return;
    }
    for (const auto &n : notifications) {
        const auto notification = n.toObject();
        if (notification["read"].toBool()) {
            continue;
        }
        auto connectionNotifications = m_oldNotifications.value(connectionId);
        if (connectionNotifications.contains(notification["event"].toObject()["event_id"].toString())) {
            continue;
        }
        connectionNotifications += notification["event"].toObject()["event_id"].toString();
        m_oldNotifications[connectionId] = connectionNotifications;

        auto room = connection->room(notification["room_id"].toString());
        if (shouldPostNotification(connection, n)) {
            // The room might have been deleted (for example rejected invitation).
            auto sender = room->user(notification["event"].toObject()["sender"].toString());

            QString body;

            if (notification["event"].toObject()["type"].toString() == "org.matrix.msc3381.poll.start") {
                body = notification["event"]
                           .toObject()["content"]
                           .toObject()["org.matrix.msc3381.poll.start"]
                           .toObject()["question"]
                           .toObject()["body"]
                           .toString();
            } else {
                body = notification["event"].toObject()["content"].toObject()["body"].toString();
            }

            if (notification["event"]["type"] == "m.room.encrypted") {
                auto decrypted = connection->decryptNotification(notification);
                body = decrypted["content"].toObject()["body"].toString();
                if (body.isEmpty()) {
                    body = i18n("Encrypted Message");
                }
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
                             notification["event"].toObject()["event_id"].toString(),
                             true);
        }
    }
}

bool NotificationsManager::shouldPostNotification(QPointer<Quotient::Connection> connection, const QJsonValue &notification)
{
    if (connection == nullptr) {
        return false;
    }

    auto room = connection->room(notification["room_id"].toString());
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
    qint64 timestamp = notification["ts"].toDouble();
    if (timestamp < m_initialTimestamp[connection->user()->id()]) {
        return false;
    }

    return true;
}

void NotificationsManager::postNotification(NeoChatRoom *room,
                                            const QString &sender,
                                            const QString &text,
                                            const QImage &icon,
                                            const QString &replyEventId,
                                            bool canReply)
{
    const QString roomId = room->id();
    KNotification *notification = m_notifications.value(roomId);
    if (!notification) {
        notification = new KNotification("message");
        m_notifications.insert(roomId, notification);
        connect(notification, &KNotification::closed, this, [this, roomId] {
            m_notifications.remove(roomId);
        });
    }

    QString entry;
    if (sender == room->displayName()) {
        notification->setTitle(sender);
        entry = text.toHtmlEscaped();
    } else {
        notification->setTitle(room->displayName());
        entry = i18n("%1: %2", sender, text.toHtmlEscaped());
    }

    notification->setText(notification->text() + '\n' + entry);
    notification->setPixmap(createNotificationImage(icon, room));

    notification->setDefaultAction(i18n("Open NeoChat in this room"));
    connect(notification, &KNotification::defaultActivated, this, [notification, room]() {
        WindowController::instance().showAndRaiseWindow(notification->xdgActivationToken());
        if (!room) {
            return;
        }
        if (room->localUser()->id() != Controller::instance().activeConnection()->userId()) {
            Controller::instance().setActiveConnection(Controller::instance().accounts().get(room->localUser()->id()));
        }
        RoomManager::instance().enterRoom(room);
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

void NotificationsManager::postInviteNotification(NeoChatRoom *room, const QString &title, const QString &sender, const QImage &icon)
{
    QPixmap img;
    img.convertFromImage(icon);
    KNotification *notification = new KNotification("invite");
    notification->setText(i18n("%1 invited you to a room", sender));
    notification->setTitle(title);
    notification->setPixmap(createNotificationImage(icon, nullptr));
    notification->setFlags(KNotification::Persistent);
    notification->setDefaultAction(i18n("Open this invitation in NeoChat"));
    connect(notification, &KNotification::defaultActivated, this, [notification, room]() {
        WindowController::instance().showAndRaiseWindow(notification->xdgActivationToken());
        notification->close();
        RoomManager::instance().enterRoom(room);
    });
    notification->setActions({i18n("Accept Invitation"), i18n("Reject Invitation")});
    connect(notification, &KNotification::action1Activated, this, [room, notification]() {
        if (!room) {
            return;
        }
        room->acceptInvitation();
        notification->close();
    });
    connect(notification, &KNotification::action2Activated, this, [room, notification]() {
        if (!room) {
            return;
        }
        RoomManager::instance().leaveRoom(room);
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
    m_invitations.insert(room->id(), notification);
}

void NotificationsManager::clearInvitationNotification(const QString &roomId)
{
    if (m_invitations.contains(roomId)) {
        m_invitations[roomId]->close();
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
