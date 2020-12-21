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
#include "controller.h"

NotificationsManager &NotificationsManager::instance()
{
    static NotificationsManager _instance;
    return _instance;
}

NotificationsManager::NotificationsManager(QObject *parent)
    : QObject(parent)
{
}

void NotificationsManager::postNotification(NeoChatRoom *room, const QString &roomName, const QString &sender, const QString &text, const QImage &icon)
{
    if (!NeoChatConfig::self()->showNotifications()) {
        return;
    }

    QPixmap img;
    img.convertFromImage(icon);
    KNotification *notification = new KNotification("message");

    if (sender == roomName) {
        notification->setTitle(sender);
    } else {
        notification->setTitle(i18n("%1 (%2)", sender, roomName));
    }

    notification->setText(text.toHtmlEscaped());
    notification->setPixmap(img);

    notification->setDefaultAction(i18n("Open NeoChat in this room"));
    connect(notification, &KNotification::defaultActivated, this, [this, room]() {
        Q_EMIT Controller::instance().openRoom(room);
        Q_EMIT Controller::instance().showWindow();
    });

    notification->sendEvent();

    m_notifications.insert(room->id(), notification);
}
