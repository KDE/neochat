// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
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
    enum Action {
        Unknown = 0,
        Off,
        On,
        Noisy,
        Highlight,
        NoisyHighlight,
    };
    Q_ENUM(Action);
};

class NotificationsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool globalNotificationsEnabled MEMBER m_globalNotificationsEnabled WRITE setGlobalNotificationsEnabled NOTIFY globalNotificationsEnabledChanged)
    Q_PROPERTY(bool globalNotificationsSet MEMBER m_globalNotificationsSet NOTIFY globalNotificationsSetChanged)
    Q_PROPERTY(PushNotificationAction::Action oneToOneNotificationAction MEMBER m_oneToOneNotificationAction WRITE setOneToOneNotificationAction NOTIFY
                   oneToOneNotificationActionChanged)
    Q_PROPERTY(PushNotificationAction::Action encryptedOneToOneNotificationAction MEMBER m_encryptedOneToOneNotificationAction WRITE
                   setEncryptedOneToOneNotificationAction NOTIFY encryptedOneToOneNotificationActionChanged)
    Q_PROPERTY(PushNotificationAction::Action groupChatNotificationAction MEMBER m_groupChatNotificationAction WRITE setGroupChatNotificationAction NOTIFY
                   groupChatNotificationActionChanged)
    Q_PROPERTY(PushNotificationAction::Action encryptedGroupChatNotificationAction MEMBER m_encryptedGroupChatNotificationAction WRITE
                   setEncryptedGroupChatNotificationAction NOTIFY encryptedGroupChatNotificationActionChanged)
    Q_PROPERTY(PushNotificationAction::Action displayNameNotificationAction MEMBER m_displayNameNotificationAction WRITE setDisplayNameNotificationAction NOTIFY
                   displayNameNotificationActionChanged)
    Q_PROPERTY(PushNotificationAction::Action roomNotificationAction MEMBER m_roomNotificationAction WRITE setRoomNotificationAction NOTIFY
                   roomNotificationActionChanged)
    Q_PROPERTY(PushNotificationAction::Action keywordNotificationAction MEMBER m_keywordNotificationAction WRITE setKeywordNotificationAction NOTIFY
                   keywordNotificationActionChanged)
    Q_PROPERTY(PushNotificationAction::Action inviteNotificationAction MEMBER m_inviteNotificationAction WRITE setInviteNotificationAction NOTIFY
                   inviteNotificationActionChanged)
    Q_PROPERTY(PushNotificationAction::Action callInviteNotificationAction MEMBER m_callInviteNotificationAction WRITE setCallInviteNotificationAction NOTIFY
                   callInviteNotificationActionChanged)
    Q_PROPERTY(PushNotificationAction::Action tombstoneNotificationAction MEMBER m_tombstoneNotificationAction WRITE setTombstoneNotificationAction NOTIFY
                   tombstoneNotificationActionChanged)

public:
    static NotificationsManager &instance();

    Q_INVOKABLE void
    postNotification(NeoChatRoom *room, const QString &sender, const QString &text, const QImage &icon, const QString &replyEventId, bool canReply);
    void postInviteNotification(NeoChatRoom *room, const QString &title, const QString &sender, const QImage &icon);

    void clearInvitationNotification(const QString &roomId);

    void initializeKeywordNotificationAction();
    void deactivateKeywordNotificationAction();
    QVector<QVariant> getKeywordNotificationActions();

private:
    NotificationsManager(QObject *parent = nullptr);

    QMultiMap<QString, KNotification *> m_notifications;
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
