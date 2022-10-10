// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "notificationsmanager.h"

#include <memory>

#include <QImage>

#include <KLocalizedString>
#include <KNotification>
#include <KNotificationReplyAction>

#include <connection.h>
#include <csapi/pushrules.h>
#include <jobs/basejob.h>
#include <user.h>

#include "actionshandler.h"
#include "controller.h"
#include "neochatconfig.h"
#include "neochatroom.h"
#include "roommanager.h"
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
    // Can't connect the signal up until the active connection has been established by the controller
    connect(&Controller::instance(), &Controller::activeConnectionChanged, this, [this]() {
        connect(Controller::instance().activeConnection(), &Connection::accountDataChanged, this, &NotificationsManager::updateGlobalNotificationsEnabled);
    });
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
        WindowController::instance().showAndRaiseWindow(notification->xdgActivationToken());
    });

    if (canReply) {
        std::unique_ptr<KNotificationReplyAction> replyAction(new KNotificationReplyAction(i18n("Reply")));
        replyAction->setPlaceholderText(i18n("Reply..."));
        connect(replyAction.get(), &KNotificationReplyAction::replied, this, [room, replyEventId](const QString &text) {
            room->postMessage(text, markdownToHTML(text), RoomMessageEvent::MsgType::Text, replyEventId, QString());
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
    notification->setFlags(KNotification::Persistent);
    notification->setDefaultAction(i18n("Open this invitation in NeoChat"));
    connect(notification, &KNotification::defaultActivated, this, [=]() {
        notification->close();
        RoomManager::instance().enterRoom(room);
        WindowController::instance().showAndRaiseWindow(notification->xdgActivationToken());
    });
    notification->setActions({i18n("Accept Invitation"), i18n("Reject Invitation")});
    connect(notification, &KNotification::action1Activated, this, [room, notification]() {
        room->acceptInvitation();
        notification->close();
    });
    connect(notification, &KNotification::action2Activated, this, [room, notification]() {
        RoomManager::instance().leaveRoom(room);
        notification->close();
    });
    connect(notification, &KNotification::closed, this, [this, room]() {
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

/**
 * The master push rule sets all notifications to off when enabled
 * see https://spec.matrix.org/v1.3/client-server-api/#default-override-rules
 * therefore to enable push rules the master rule needs to be disabled and vice versa
 */
void NotificationsManager::setGlobalNotificationsEnabled(bool enabled)
{
    using namespace Quotient;

    auto job = Controller::instance().activeConnection()->callApi<IsPushRuleEnabledJob>("global", "override", ".m.rule.master");
    connect(job, &BaseJob::success, this, [this, job, enabled]() {
        if (job->enabled() == enabled) {
            Controller::instance().activeConnection()->callApi<SetPushRuleEnabledJob>("global", "override", ".m.rule.master", !enabled);
            m_globalNotificationsEnabled = enabled;
            Q_EMIT globalNotificationsEnabledChanged(m_globalNotificationsEnabled);
        }
    });
}

void NotificationsManager::updateGlobalNotificationsEnabled(QString type)
{
    if (type != "m.push_rules") {
        return;
    }

    QJsonObject accountData = Controller::instance().activeConnection()->accountDataJson("m.push_rules");
    QJsonArray overrideRuleArray = accountData.value("global").toObject().value("override").toArray();

    for (const auto &i : overrideRuleArray) {
        QJsonObject overrideRule = i.toObject();
        if (overrideRule.value("rule_id") == ".m.rule.master") {
            bool ruleEnabled = overrideRule.value("enabled").toBool();
            m_globalNotificationsEnabled = !ruleEnabled;
            NeoChatConfig::self()->setShowNotifications(m_globalNotificationsEnabled);
            Q_EMIT globalNotificationsEnabledChanged(m_globalNotificationsEnabled);
        }
    }
}
