// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QImage>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QString>
#include <Quotient/csapi/notifications.h>
#include <Quotient/jobs/basejob.h>

namespace Quotient
{
class Connection;
}

class KNotification;
class NeoChatRoom;

class PushNotificationAction : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Defines the global push notification actions.
     */
    enum Action {
        Unknown = 0, /**< The action has not yet been obtained from the server. */
        Off, /**< No push notifications are to be sent. */
        On, /**< Push notifications are on. */
        Noisy, /**< Push notifications are on, also trigger a notification sound. */
        Highlight, /**< Push notifications are on, also the event should be highlighted in chat. */
        NoisyHighlight, /**< Push notifications are on, also trigger a notification sound and highlight in chat. */
    };
    Q_ENUM(Action)
};

/**
 * @class NotificationsManager
 *
 * This class is responsible for managing notifications.
 *
 * This includes sending native notifications on mobile or desktop if available as
 * well as managing the push notification rules for the current active account.
 *
 * @note Matrix manages push notifications centrally for an account and these are
 *       stored on the home server. This is to allow a users settings to move between clients.
 *       See https://spec.matrix.org/v1.3/client-server-api/#push-rules for more
 *       details.
 */
class NotificationsManager : public QObject
{
    Q_OBJECT

public:
    static NotificationsManager &instance();

    /**
     * @brief Display a native notification for an message.
     */
    Q_INVOKABLE void
    postNotification(NeoChatRoom *room, const QString &sender, const QString &text, const QImage &icon, const QString &replyEventId, bool canReply);

    /**
     * @brief Display a native notification for an invite.
     */
    void postInviteNotification(NeoChatRoom *room, const QString &title, const QString &sender, const QImage &icon);

    /**
     * @brief Clear an existing invite notification for the given room.
     *
     * Nothing happens if the given room doesn't have an invite notification.
     */
    void clearInvitationNotification(const QString &roomId);

    /**
     * @brief Handle the notifications for the given connection.
     */
    void handleNotifications(QPointer<Quotient::Connection> connection);

private:
    explicit NotificationsManager(QObject *parent = nullptr);

    QHash<QString, qint64> m_initialTimestamp;
    QHash<QString, QStringList> m_oldNotifications;

    QStringList m_connActiveJob;

    bool shouldPostNotification(QPointer<Quotient::Connection> connection, const QJsonValue &notification);

    QHash<QString, KNotification *> m_notifications;
    QHash<QString, QPointer<KNotification>> m_invitations;

private Q_SLOTS:
    void processNotificationJob(QPointer<Quotient::Connection> connection, Quotient::GetNotificationsJob *job, bool initialization);

private:
    QPixmap createNotificationImage(const QImage &icon, NeoChatRoom *room);
};
