// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "notificationsmanager.h"

#include <memory>

#include <QJsonArray>

#include <KLocalizedString>
#include <KNotification>
#include <KNotificationReplyAction>

#ifdef QUOTIENT_07
#include <accountregistry.h>
#else
#include "neochataccountregistry.h"
#endif

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
        connect(Controller::instance().activeConnection(), &Connection::accountDataChanged, this, &NotificationsManager::updateNotificationRules);
        // Ensure that the push rule states are retrieved after the connection is changed
        updateNotificationRules("m.push_rules");
    });
}

void NotificationsManager::postNotification(NeoChatRoom *room,
                                            const QString &sender,
                                            const QString &text,
                                            const QImage &icon,
                                            const QString &replyEventId,
                                            bool canReply)
{
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
        if (room->localUser()->id() != Controller::instance().activeConnection()->userId()) {
#ifdef QUOTIENT_07
            Controller::instance().setActiveConnection(Accounts.get(room->localUser()->id()));
#else
            Controller::instance().setActiveConnection(AccountRegistry::instance().get(room->localUser()->id()));
#endif
        }
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
    setNotificationRuleEnabled("override", ".m.rule.master", !enabled);
}

void NotificationsManager::setOneToOneNotificationAction(PushNotificationAction::Action action)
{
    setNotificationRuleActions("underride", ".m.rule.room_one_to_one", action);
}

void NotificationsManager::setEncryptedOneToOneNotificationAction(PushNotificationAction::Action action)
{
    setNotificationRuleActions("underride", ".m.rule.encrypted_room_one_to_one", action);
}

void NotificationsManager::setGroupChatNotificationAction(PushNotificationAction::Action action)
{
    setNotificationRuleActions("underride", ".m.rule.message", action);
}

void NotificationsManager::setEncryptedGroupChatNotificationAction(PushNotificationAction::Action action)
{
    setNotificationRuleActions("underride", ".m.rule.encrypted", action);
}

/*
 * .m.rule.contains_display_name is an override rule so it needs to be disabled when off
 * so that other rules can match the message if they apply.
 */
void NotificationsManager::setDisplayNameNotificationAction(PushNotificationAction::Action action)
{
    if (action == PushNotificationAction::Off) {
        setNotificationRuleEnabled("override", ".m.rule.contains_display_name", false);
    } else {
        setNotificationRuleActions("override", ".m.rule.contains_display_name", action);
        setNotificationRuleEnabled("override", ".m.rule.contains_display_name", true);
    }
}

/*
 * .m.rule.roomnotif is an override rule so it needs to be disabled when off
 * so that other rules can match the message if they apply.
 */
void NotificationsManager::setRoomNotificationAction(PushNotificationAction::Action action)
{
    if (action == PushNotificationAction::Off) {
        setNotificationRuleEnabled("override", ".m.rule.roomnotif", false);
    } else {
        setNotificationRuleActions("override", ".m.rule.roomnotif", action);
        setNotificationRuleEnabled("override", ".m.rule.roomnotif", true);
    }
}

void NotificationsManager::initializeKeywordNotificationAction()
{
    m_keywordNotificationAction = PushNotificationAction::Highlight;
    Q_EMIT keywordNotificationActionChanged(m_keywordNotificationAction);
}

void NotificationsManager::deactivateKeywordNotificationAction()
{
    m_keywordNotificationAction = PushNotificationAction::Off;
    Q_EMIT keywordNotificationActionChanged(m_keywordNotificationAction);
}

QVector<QVariant> NotificationsManager::getKeywordNotificationActions()
{
    return toActions(m_keywordNotificationAction);
}

void NotificationsManager::setKeywordNotificationAction(PushNotificationAction::Action action)
{
    // Unlike the other rules this needs to be set here for the case where there are no keyords.
    m_keywordNotificationAction = action;
    Q_EMIT keywordNotificationActionChanged(m_keywordNotificationAction);

    const QJsonObject accountData = Controller::instance().activeConnection()->accountDataJson("m.push_rules");
    const QJsonArray contentRuleArray = accountData["global"].toObject()["content"].toArray();
    for (const auto &i : contentRuleArray) {
        const QJsonObject contentRule = i.toObject();
        if (contentRule["rule_id"].toString()[0] != '.') {
            setNotificationRuleActions("content", contentRule["rule_id"].toString(), action);
        }
    }
}

/*
 * .m.rule.invite_for_me is an override rule so it needs to be disabled when off
 * so that other rules can match the message if they apply.
 */
void NotificationsManager::setInviteNotificationAction(PushNotificationAction::Action action)
{
    if (action == PushNotificationAction::Off) {
        setNotificationRuleEnabled("override", ".m.rule.invite_for_me", false);
    } else {
        setNotificationRuleActions("override", ".m.rule.invite_for_me", action);
        setNotificationRuleEnabled("override", ".m.rule.invite_for_me", true);
    }
}

void NotificationsManager::setCallInviteNotificationAction(PushNotificationAction::Action action)
{
    setNotificationRuleActions("underride", ".m.rule.call", action);
}

/*
 * .m.rule.tombstone is an override rule so it needs to be disabled when off
 * so that other rules can match the message if they apply.
 */
void NotificationsManager::setTombstoneNotificationAction(PushNotificationAction::Action action)
{
    if (action == PushNotificationAction::Off) {
        setNotificationRuleEnabled("override", ".m.rule.tombstone", false);
    } else {
        setNotificationRuleActions("override", ".m.rule.tombstone", action);
        setNotificationRuleEnabled("override", ".m.rule.tombstone", true);
    }
}

void NotificationsManager::updateNotificationRules(const QString &type)
{
    if (type != "m.push_rules") {
        return;
    }

    const QJsonObject accountData = Controller::instance().activeConnection()->accountDataJson("m.push_rules");

    // Update override rules
    const QJsonArray overrideRuleArray = accountData["global"].toObject()["override"].toArray();
    for (const auto &i : overrideRuleArray) {
        const QJsonObject overrideRule = i.toObject();
        if (overrideRule["rule_id"] == ".m.rule.master") {
            bool ruleEnabled = overrideRule["enabled"].toBool();
            m_globalNotificationsEnabled = !ruleEnabled;
            if (!m_globalNotificationsSet) {
                m_globalNotificationsSet = true;
            }
            Q_EMIT globalNotificationsEnabledChanged(m_globalNotificationsEnabled);
        }

        const PushNotificationAction::Action action = toAction(overrideRule);

        if (overrideRule["rule_id"] == ".m.rule.contains_display_name") {
            m_displayNameNotificationAction = action;
            Q_EMIT displayNameNotificationActionChanged(m_displayNameNotificationAction);
        } else if (overrideRule["rule_id"] == ".m.rule.roomnotif") {
            m_roomNotificationAction = action;
            Q_EMIT roomNotificationActionChanged(m_roomNotificationAction);
        } else if (overrideRule["rule_id"] == ".m.rule.invite_for_me") {
            m_inviteNotificationAction = action;
            Q_EMIT inviteNotificationActionChanged(m_inviteNotificationAction);
        } else if (overrideRule["rule_id"] == ".m.rule.tombstone") {
            m_tombstoneNotificationAction = action;
            Q_EMIT tombstoneNotificationActionChanged(m_tombstoneNotificationAction);
        }
    }

    // Update content rules
    const QJsonArray contentRuleArray = accountData["global"].toObject()["content"].toArray();
    PushNotificationAction::Action keywordAction = PushNotificationAction::Unknown;
    for (const auto &i : contentRuleArray) {
        const QJsonObject contentRule = i.toObject();
        const PushNotificationAction::Action action = toAction(contentRule);
        bool actionMismatch = false;

        if (contentRule["rule_id"].toString()[0] != '.' && !actionMismatch) {
            if (keywordAction == PushNotificationAction::Unknown) {
                keywordAction = action;
                m_keywordNotificationAction = action;
                Q_EMIT keywordNotificationActionChanged(m_keywordNotificationAction);
            } else if (action != keywordAction) {
                actionMismatch = true;
                m_keywordNotificationAction = PushNotificationAction::On;
                Q_EMIT keywordNotificationActionChanged(m_keywordNotificationAction);
            }
        }
    }
    // If there are no keywords set the state to off, this is the only time it'll be in the off state
    if (keywordAction == PushNotificationAction::Unknown) {
        m_keywordNotificationAction = PushNotificationAction::Off;
        Q_EMIT keywordNotificationActionChanged(m_keywordNotificationAction);
    }

    // Update underride rules
    const QJsonArray underrideRuleArray = accountData["global"].toObject()["underride"].toArray();
    for (const auto &i : underrideRuleArray) {
        const QJsonObject underrideRule = i.toObject();
        const PushNotificationAction::Action action = toAction(underrideRule);

        if (underrideRule["rule_id"] == ".m.rule.room_one_to_one") {
            m_oneToOneNotificationAction = action;
            Q_EMIT oneToOneNotificationActionChanged(m_oneToOneNotificationAction);
        } else if (underrideRule["rule_id"] == ".m.rule.encrypted_room_one_to_one") {
            m_encryptedOneToOneNotificationAction = action;
            Q_EMIT encryptedOneToOneNotificationActionChanged(m_encryptedOneToOneNotificationAction);
        } else if (underrideRule["rule_id"] == ".m.rule.message") {
            m_groupChatNotificationAction = action;
            Q_EMIT groupChatNotificationActionChanged(m_groupChatNotificationAction);
        } else if (underrideRule["rule_id"] == ".m.rule.encrypted") {
            m_encryptedGroupChatNotificationAction = action;
            Q_EMIT encryptedGroupChatNotificationActionChanged(m_encryptedGroupChatNotificationAction);
        } else if (underrideRule["rule_id"] == ".m.rule.call") {
            m_callInviteNotificationAction = action;
            Q_EMIT callInviteNotificationActionChanged(m_callInviteNotificationAction);
        }
    }
}

void NotificationsManager::setNotificationRuleEnabled(const QString &kind, const QString &ruleId, bool enabled)
{
    auto job = Controller::instance().activeConnection()->callApi<IsPushRuleEnabledJob>("global", kind, ruleId);
    connect(job, &BaseJob::success, this, [job, kind, ruleId, enabled]() {
        if (job->enabled() != enabled) {
            Controller::instance().activeConnection()->callApi<SetPushRuleEnabledJob>("global", kind, ruleId, enabled);
        }
    });
}

void NotificationsManager::setNotificationRuleActions(const QString &kind, const QString &ruleId, PushNotificationAction::Action action)
{
    QVector<QVariant> actions;
    if (ruleId == ".m.rule.call") {
        actions = toActions(action, "ring");
    } else {
        actions = toActions(action);
    }

    Controller::instance().activeConnection()->callApi<SetPushRuleActionsJob>("global", kind, ruleId, actions);
}

PushNotificationAction::Action NotificationsManager::toAction(const QJsonObject &rule)
{
    const QJsonArray actions = rule["actions"].toArray();
    bool isNoisy = false;
    bool highlightEnabled = false;
    const bool enabled = rule["enabled"].toBool();
    for (const auto &i : actions) {
        QJsonObject action = i.toObject();
        if (action["set_tweak"].toString() == "sound") {
            isNoisy = true;
        } else if (action["set_tweak"].toString() == "highlight") {
            if (action["value"].toString() != "false") {
                highlightEnabled = true;
            }
        }
    }

    if (!enabled) {
        return PushNotificationAction::Off;
    }

    if (actions[0] == "notify") {
        if (isNoisy && highlightEnabled) {
            return PushNotificationAction::NoisyHighlight;
        } else if (isNoisy) {
            return PushNotificationAction::Noisy;
        } else if (highlightEnabled) {
            return PushNotificationAction::Highlight;
        } else {
            return PushNotificationAction::On;
        }
    } else {
        return PushNotificationAction::Off;
    }
}

QVector<QVariant> NotificationsManager::toActions(PushNotificationAction::Action action, const QString &sound)
{
    // The caller should never try to set the state to unknown.
    // It exists only as a default state to diable the settings options until the actual state is retrieved from the server.
    if (action == PushNotificationAction::Unknown) {
        Q_ASSERT(false);
        return QVector<QVariant>();
    }

    QVector<QVariant> actions;

    if (action != PushNotificationAction::Off) {
        actions.append("notify");
    } else {
        actions.append("dont_notify");
    }
    if (action == PushNotificationAction::Noisy || action == PushNotificationAction::NoisyHighlight) {
        QJsonObject soundTweak;
        soundTweak.insert("set_tweak", "sound");
        soundTweak.insert("value", sound);
        actions.append(soundTweak);
    }
    if (action == PushNotificationAction::Highlight || action == PushNotificationAction::NoisyHighlight) {
        QJsonObject highlightTweak;
        highlightTweak.insert("set_tweak", "highlight");
        actions.append(highlightTweak);
    }

    return actions;
}
