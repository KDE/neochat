// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QImage>
#include <QMap>
#include <QObject>
#include <QString>

#include <KNotification>

#include "neochatconfig.h"
#include "neochatroom.h"

class NotificationsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool globalNotificationsEnabled MEMBER m_globalNotificationsEnabled WRITE setGlobalNotificationsEnabled NOTIFY globalNotificationsEnabledChanged)

public:
    static NotificationsManager &instance();

    Q_INVOKABLE void
    postNotification(NeoChatRoom *room, const QString &sender, const QString &text, const QImage &icon, const QString &replyEventId, bool canReply);
    void postInviteNotification(NeoChatRoom *room, const QString &title, const QString &sender, const QImage &icon);

    void clearInvitationNotification(const QString &roomId);

    Q_INVOKABLE void setGlobalNotificationsEnabled(bool enabled);

private:
    NotificationsManager(QObject *parent = nullptr);

    QMultiMap<QString, KNotification *> m_notifications;
    QHash<QString, QPointer<KNotification>> m_invitations;

    bool m_globalNotificationsEnabled;

private Q_SLOTS:
    void updateGlobalNotificationsEnabled(QString type);

Q_SIGNALS:
    void globalNotificationsEnabledChanged(bool newState);
};
