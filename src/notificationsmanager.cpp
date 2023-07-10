// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "notificationsmanager.h"

#include <memory>

#include <QGuiApplication>

#include <KLocalizedString>
#include <KNotification>
#include <KNotificationReplyAction>

#ifdef QUOTIENT_07
#include <accountregistry.h>
#else
#include "neochataccountregistry.h"
#endif

#include <connection.h>
#include <csapi/notifications.h>
#include <csapi/pushrules.h>
#include <jobs/basejob.h>
#include <user.h>

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

#ifdef QUOTIENT_07
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
#endif

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
#ifdef Quotient_E2EE_ENABLED
                auto decrypted = connection->decryptNotification(notification);
                body = decrypted["content"].toObject()["body"].toString();
#endif
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
    notification->setPixmap(QPixmap::fromImage(icon));

    notification->setDefaultAction(i18n("Open NeoChat in this room"));
    connect(notification, &KNotification::defaultActivated, this, [notification, room]() {
        WindowController::instance().showAndRaiseWindow(notification->xdgActivationToken());
        if (!room) {
            return;
        }
        if (room->localUser()->id() != Controller::instance().activeConnection()->userId()) {
#ifdef QUOTIENT_07
            Controller::instance().setActiveConnection(Accounts.get(room->localUser()->id()));
#else
            Controller::instance().setActiveConnection(AccountRegistry::instance().get(room->localUser()->id()));
#endif
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
    notification->setPixmap(img);
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
