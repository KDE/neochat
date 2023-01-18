// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QImage>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QJsonObject>

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
    Q_ENUM(Action);
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

    /**
     * @brief The global notification state.
     *
     * If this rule is set to off all push notifications are disabled regardless
     * of other settings.
     */
    Q_PROPERTY(bool globalNotificationsEnabled MEMBER m_globalNotificationsEnabled WRITE setGlobalNotificationsEnabled NOTIFY globalNotificationsEnabledChanged)

    /**
     * @brief Whether the global notification state has been retrieved from the server.
     *
     * This is different to the others below as globalNotificationsEnabled is only
     * a bool rather than a PushNotificationAction::Action so a separate property
     * is required to track if the state has been retrieved from the server.
     *
     * @sa globalNotificationsEnabled, PushNotificationAction::Action
     */
    Q_PROPERTY(bool globalNotificationsSet MEMBER m_globalNotificationsSet NOTIFY globalNotificationsSetChanged)

    /**
     * @brief The notification action for direct chats.
     *
     * @note Encrypted direct chats have a separate setting.
     *
     * @sa encryptedOneToOneNotificationAction
     */
    Q_PROPERTY(PushNotificationAction::Action oneToOneNotificationAction MEMBER m_oneToOneNotificationAction WRITE setOneToOneNotificationAction NOTIFY
                   oneToOneNotificationActionChanged)

    /**
     * @brief The notification action for encrypted direct chats.
     */
    Q_PROPERTY(PushNotificationAction::Action encryptedOneToOneNotificationAction MEMBER m_encryptedOneToOneNotificationAction WRITE
                   setEncryptedOneToOneNotificationAction NOTIFY encryptedOneToOneNotificationActionChanged)

    /**
     * @brief The notification action for group chats.
     *
     * @note Encrypted group chats have a separate setting.
     *
     * @sa encryptedGroupChatNotificationAction
     */
    Q_PROPERTY(PushNotificationAction::Action groupChatNotificationAction MEMBER m_groupChatNotificationAction WRITE setGroupChatNotificationAction NOTIFY
                   groupChatNotificationActionChanged)

    /**
     * @brief The notification action for encrypted group chats.
     */
    Q_PROPERTY(PushNotificationAction::Action encryptedGroupChatNotificationAction MEMBER m_encryptedGroupChatNotificationAction WRITE
                   setEncryptedGroupChatNotificationAction NOTIFY encryptedGroupChatNotificationActionChanged)

    /**
     * @brief The notification action for events containing the local user's display name.
     */
    Q_PROPERTY(PushNotificationAction::Action displayNameNotificationAction MEMBER m_displayNameNotificationAction WRITE setDisplayNameNotificationAction NOTIFY
                   displayNameNotificationActionChanged)

    /**
     * @brief The notification action for room events.
     */
    Q_PROPERTY(PushNotificationAction::Action roomNotificationAction MEMBER m_roomNotificationAction WRITE setRoomNotificationAction NOTIFY
                   roomNotificationActionChanged)

    /**
     * @brief The notification action for keyword push rules.
     */
    Q_PROPERTY(PushNotificationAction::Action keywordNotificationAction MEMBER m_keywordNotificationAction WRITE setKeywordNotificationAction NOTIFY
                   keywordNotificationActionChanged)

    /**
     * @brief The notification action for invites to chats.
     */
    Q_PROPERTY(PushNotificationAction::Action inviteNotificationAction MEMBER m_inviteNotificationAction WRITE setInviteNotificationAction NOTIFY
                   inviteNotificationActionChanged)

    /**
     * @brief The notification action for voice calls.
     */
    Q_PROPERTY(PushNotificationAction::Action callInviteNotificationAction MEMBER m_callInviteNotificationAction WRITE setCallInviteNotificationAction NOTIFY
                   callInviteNotificationActionChanged)

    /**
     * @brief The notification action for room upgrade events.
     */
    Q_PROPERTY(PushNotificationAction::Action tombstoneNotificationAction MEMBER m_tombstoneNotificationAction WRITE setTombstoneNotificationAction NOTIFY
                   tombstoneNotificationActionChanged)

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
     * @brief Set the initial keyword notification action.
     *
     * This is only required if no keyword rules exist and one is created. The default
     * action is PushNotificationAction::Action::Highlight.
     *
     * @sa PushNotificationAction::Action
     */
    void initializeKeywordNotificationAction();

    /**
     * @brief Set the keyword notification action to PushNotificationAction::Action::Off.
     *
     * This is only required the last keyword rule is deleted.
     *
     * @sa PushNotificationAction::Action
     */
    void deactivateKeywordNotificationAction();

    /**
     * @brief Return the keyword notification action in a form required for the server push rule.
     */
    QVector<QVariant> getKeywordNotificationActions();

private:
    NotificationsManager(QObject *parent = nullptr);

    QHash<QString, KNotification *> m_notifications;
    QHash<QString, QPointer<KNotification>> m_invitations;

    bool m_globalNotificationsEnabled = false;
    bool m_globalNotificationsSet = false;
    PushNotificationAction::Action m_oneToOneNotificationAction = PushNotificationAction::Unknown;
    PushNotificationAction::Action m_encryptedOneToOneNotificationAction = PushNotificationAction::Unknown;
    PushNotificationAction::Action m_groupChatNotificationAction = PushNotificationAction::Unknown;
    PushNotificationAction::Action m_encryptedGroupChatNotificationAction = PushNotificationAction::Unknown;
    PushNotificationAction::Action m_displayNameNotificationAction = PushNotificationAction::Unknown;
    PushNotificationAction::Action m_roomNotificationAction = PushNotificationAction::Unknown;
    PushNotificationAction::Action m_keywordNotificationAction = PushNotificationAction::Unknown;
    PushNotificationAction::Action m_inviteNotificationAction = PushNotificationAction::Unknown;
    PushNotificationAction::Action m_callInviteNotificationAction = PushNotificationAction::Unknown;
    PushNotificationAction::Action m_tombstoneNotificationAction = PushNotificationAction::Unknown;

    void setGlobalNotificationsEnabled(bool enabled);
    void setOneToOneNotificationAction(PushNotificationAction::Action action);
    void setEncryptedOneToOneNotificationAction(PushNotificationAction::Action action);
    void setGroupChatNotificationAction(PushNotificationAction::Action action);
    void setEncryptedGroupChatNotificationAction(PushNotificationAction::Action action);
    void setDisplayNameNotificationAction(PushNotificationAction::Action action);
    void setRoomNotificationAction(PushNotificationAction::Action action);
    void setKeywordNotificationAction(PushNotificationAction::Action action);
    void setInviteNotificationAction(PushNotificationAction::Action action);
    void setCallInviteNotificationAction(PushNotificationAction::Action action);
    void setTombstoneNotificationAction(PushNotificationAction::Action action);

    void setNotificationRuleEnabled(const QString &kind, const QString &ruleId, bool enabled);
    void setNotificationRuleActions(const QString &kind, const QString &ruleId, PushNotificationAction::Action action);
    PushNotificationAction::Action toAction(const QJsonObject &rule);
    QVector<QVariant> toActions(PushNotificationAction::Action action, const QString &sound = "default");

private Q_SLOTS:
    void updateNotificationRules(const QString &type);

Q_SIGNALS:
    void globalNotificationsEnabledChanged(bool newState);
    void globalNotificationsSetChanged(bool newState);
    void oneToOneNotificationActionChanged(PushNotificationAction::Action action);
    void encryptedOneToOneNotificationActionChanged(PushNotificationAction::Action action);
    void groupChatNotificationActionChanged(PushNotificationAction::Action action);
    void encryptedGroupChatNotificationActionChanged(PushNotificationAction::Action action);
    void displayNameNotificationActionChanged(PushNotificationAction::Action action);
    void roomNotificationActionChanged(PushNotificationAction::Action action);
    void keywordNotificationActionChanged(PushNotificationAction::Action action);
    void inviteNotificationActionChanged(PushNotificationAction::Action action);
    void callInviteNotificationActionChanged(PushNotificationAction::Action action);
    void tombstoneNotificationActionChanged(PushNotificationAction::Action action);
};
