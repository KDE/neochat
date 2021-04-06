// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "notificationsmanager.h"

#include <memory>

#include <QDebug>
#include <QImage>

#include "knotifications_version.h"
#include <KLocalizedString>
#include <KNotification>
#if KNOTIFICATIONS_VERSION >= QT_VERSION_CHECK(5, 81, 0)
#include <KNotificationReplyAction>
#endif

#include "controller.h"
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

void NotificationsManager::postNotification(NeoChatRoom *room,
                                            const QString &roomName,
                                            const QString &sender,
                                            const QString &text,
                                            const QImage &icon,
                                            const QString &replyEventId)
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

#if KNOTIFICATIONS_VERSION >= QT_VERSION_CHECK(5, 81, 0)
    std::unique_ptr<KNotificationReplyAction> replyAction(new KNotificationReplyAction(i18n("Reply")));
    replyAction->setPlaceholderText(i18n("Reply..."));
    QObject::connect(replyAction.get(), &KNotificationReplyAction::replied, [room, replyEventId](const QString &text) {
        room->postMessage(text, text, RoomMessageEvent::MsgType::Text, replyEventId, QString());
    });
    notification->setReplyAction(std::move(replyAction));
#endif

    notification->sendEvent();

    m_notifications.insert(room->id(), notification);
}
