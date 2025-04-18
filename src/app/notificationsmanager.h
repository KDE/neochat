// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QImage>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QString>
#include <Quotient/csapi/notifications.h>
#include <Quotient/jobs/basejob.h>

class NeoChatConnection;
class KNotification;
class NeoChatRoom;

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
    QML_ELEMENT

public:
    explicit NotificationsManager(QObject *parent = nullptr);

    /**
     * @brief Display a native notification for an invite.
     */
    void postInviteNotification(NeoChatRoom *room);

    /**
     * @brief Clear an existing invite notification for the given room.
     *
     * Nothing happens if the given room doesn't have an invite notification.
     */
    void clearInvitationNotification(const QString &roomId);

    /**
     * @brief Display a native notification for the given push notification.
     */
    void postPushNotification(const QByteArray &message);

    /**
     * @brief Handle the notifications for the given connection.
     */
    void handleNotifications(QPointer<NeoChatConnection> connection);

private:
    QHash<QString, qint64> m_initialTimestamp;
    QHash<QString, QStringList> m_oldNotifications;

    QStringList m_connActiveJob;
    void startNotificationJob(QPointer<NeoChatConnection> connection);

    QPixmap createNotificationImage(const QImage &icon, NeoChatRoom *room);
    bool shouldPostNotification(QPointer<NeoChatConnection> connection, const QJsonValue &notification);
    void postNotification(NeoChatRoom *room,
                          const QString &sender,
                          const QString &text,
                          const QImage &icon,
                          const QString &replyEventId,
                          bool canReply,
                          qint64 timestamp);

    void doPostInviteNotification(QPointer<NeoChatRoom> room);

    QHash<QString, std::pair<qint64, KNotification *>> m_notifications;
    QHash<QString, QPointer<KNotification>> m_invitations;

    bool permissionAsked = false;

private Q_SLOTS:
    void processNotificationJob(QPointer<NeoChatConnection> connection, Quotient::GetNotificationsJob *job, bool initialization);
};
