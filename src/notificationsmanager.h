// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QImage>
#include <QMap>
#include <QObject>
#include <QString>

#include <KNotification>

#include "neochatroom.h"

class NotificationsManager : public QObject
{
    Q_OBJECT

public:
    static NotificationsManager &instance();

    Q_INVOKABLE void
    postNotification(NeoChatRoom *room, const QString &roomName, const QString &sender, const QString &text, const QImage &icon, const QString &replyEventId);
    void postInviteNotification(NeoChatRoom *room, const QString &title, const QString &sender, const QImage &icon);

private:
    NotificationsManager(QObject *parent = nullptr);

    QMultiMap<QString, KNotification *> m_notifications;
};
