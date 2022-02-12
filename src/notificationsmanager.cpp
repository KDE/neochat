// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "notificationsmanager.h"

#include <memory>

#include <QDebug>
#include <QImage>

#include "knotifications_version.h"
#include <KLocalizedString>
#include <KNotification>
#ifdef HAVE_WINDOWSYSTEM
#include <KWindowSystem>
#endif
#include <KNotificationReplyAction>

#include "controller.h"
#include "neochatconfig.h"
#include "roommanager.h"

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
                                            const QString &sender,
                                            const QString &text,
                                            const QImage &icon,
                                            const QString &replyEventId,
                                            bool canReply)
{
    if (!NeoChatConfig::self()->showNotifications()) {
        return;
    }

    QPixmap img;
    img.convertFromImage(icon);
    KNotification *notification = new KNotification("message");

    if (sender == room->displayName()) {
        notification->setTitle(sender);
    } else {
        notification->setTitle(i18n("%1 (%2)", sender, room->displayName()));
    }

    notification->setText(text.toHtmlEscaped());
    notification->setPixmap(img);

    notification->setDefaultAction(i18n("Open NeoChat in this room"));
    connect(notification, &KNotification::defaultActivated, this, [=]() {
        RoomManager::instance().enterRoom(room);
        Q_EMIT Controller::instance().showWindow();
    });

    if (canReply) {
        std::unique_ptr<KNotificationReplyAction> replyAction(new KNotificationReplyAction(i18n("Reply")));
        replyAction->setPlaceholderText(i18n("Reply..."));
        connect(replyAction.get(), &KNotificationReplyAction::replied, this, [room, replyEventId](const QString &text) {
            room->postMessage(text, room->preprocessText(text), RoomMessageEvent::MsgType::Text, replyEventId, QString());
        });
        notification->setReplyAction(std::move(replyAction));
    }

    notification->setHint(QStringLiteral("x-kde-origin-name"), room->localUser()->id());


    notification->sendEvent();

    m_notifications.insert(room->id(), notification);
}

void NotificationsManager::postInviteNotification(NeoChatRoom *room, const QString &title, const QString &sender, const QImage &icon)
{
    if (!NeoChatConfig::self()->showNotifications()) {
        return;
    }
    QPixmap img;
    img.convertFromImage(icon);
    KNotification *notification = new KNotification("invite");
    notification->setText(i18n("%1 invited you to a room", sender));
    notification->setTitle(title);
    notification->setPixmap(img);
    notification->setDefaultAction(i18n("Open this invitation in NeoChat"));
    connect(notification, &KNotification::defaultActivated, this, [=]() {
#if defined(HAVE_WINDOWSYSTEM) && KNOTIFICATIONS_VERSION >= QT_VERSION_CHECK(5, 90, 0)
        KWindowSystem::setCurrentXdgActivationToken(notification->xdgActivationToken());
#endif
        RoomManager::instance().enterRoom(room);
        Q_EMIT Controller::instance().showWindow();
    });
    notification->setActions({i18n("Accept Invitation"), i18n("Reject Invitation")});
    connect(notification, &KNotification::action1Activated, this, [room]() {
        room->acceptInvitation();
    });
    connect(notification, &KNotification::action2Activated, this, [room]() {
        RoomManager::instance().leaveRoom(room);
    });

    notification->setHint(QStringLiteral("x-kde-origin-name"), room->localUser()->id());

    notification->sendEvent();
    m_notifications.insert(room->id(), notification);
}
