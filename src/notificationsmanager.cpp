/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "notificationsmanager.h"

#include <QDebug>
#include <QImage>

#include <KLocalizedString>
#include <KNotification>

#include "neochatconfig.h"

NotificationsManager &NotificationsManager::instance()
{
    static NotificationsManager _instance;
    return _instance;
}


NotificationsManager::NotificationsManager(QObject *parent)
    : QObject(parent)
{
}

void NotificationsManager::postNotification(const QString &roomid, const QString &eventid, const QString &roomname, const QString &sender, const QString &text, const QImage &icon)
{
    if(!NeoChatConfig::self()->showNotifications()) {
        return;
    }

    QPixmap img;
    img.convertFromImage(icon);
    KNotification *notification = new KNotification("message");
    notification->setTitle(i18n("%1 (%2)", sender, roomname));
    notification->setText(text);
    notification->setPixmap(img);
    notification->sendEvent();

    m_notifications.insert(roomid, notification);
}
