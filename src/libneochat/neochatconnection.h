// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QCache>
#include <QObject>
#include <QQmlEngine>

#include <QCoroTask>
#include <Quotient/connection.h>

#include <Quotient/keyimport.h>

#include "enums/messagetype.h"
#include "enums/pushrule.h"
#include "linkpreviewer.h"

class NeoChatConnection : public Quotient::Connection
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The account label for this account.
     *
     * Account labels are a concept specific to NeoChat, allowing accounts to be
     * labelled, e.g. for "Work", "Private", etc.
     *
     * Set to an empty string to remove the label.
     */
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)

    /**
     * @brief Whether an identity server is configured.
     */
    Q_PROPERTY(bool hasIdentityServer READ hasIdentityServer NOTIFY identityServerChanged)

    /**
     * @brief The identity server URL as a string for showing in a UI.
     *
     * Will return the string "No identity server configured" if no identity
     * server configured. Otherwise it returns the URL as a string.
     */
    Q_PROPERTY(QString identityServer READ identityServerUIString NOTIFY identityServerChanged)

    /**
     * @brief The total number of notifications for all direct chats.
     */
    Q_PROPERTY(qsizetype directChatNotifications READ directChatNotifications NOTIFY directChatNotificationsChanged)

    /**
     * @brief Whether any direct chats have highlight notifications.
     */
    Q_PROPERTY(bool directChatsHaveHighlightNotifications READ directChatsHaveHighlightNotifications NOTIFY directChatsHaveHighlightNotificationsChanged)

    /**
     * @brief The total number of notifications for all rooms in the home tab.
     */
    Q_PROPERTY(qsizetype homeNotifications READ homeNotifications NOTIFY homeNotificationsChanged)

    /**
     * @brief Whether any of the rooms in the home tab have highlight notifications.
     */
    Q_PROPERTY(bool homeHaveHighlightNotifications READ homeHaveHighlightNotifications NOTIFY homeHaveHighlightNotificationsChanged)

    /**
     * @brief The number of invites to 1-on-1 direct chats.
     */
    Q_PROPERTY(qsizetype directChatInvites READ directChatInvites NOTIFY directChatInvitesChanged)

    /**
     * @brief The number of pending, normal room invites.
     */
    Q_PROPERTY(qsizetype roomInvites READ roomInvites NOTIFY roomInvitesChanged)

    /**
     * @brief Whether the server supports querying a user's mutual rooms.
     */
    Q_PROPERTY(bool canCheckMutualRooms READ canCheckMutualRooms NOTIFY canCheckMutualRoomsChanged)

    /**
     * @brief Whether the server supports erasing user data when deactivating the account. This checks for MSC4025.
     */
    Q_PROPERTY(bool canEraseData READ canEraseData NOTIFY canEraseDataChanged)

    /**
     * @brief Whether this build of NeoChat supports push notifications via KUnifiedPush.
     */
    Q_PROPERTY(bool pushNotificationsAvailable READ pushNotificationsAvailable CONSTANT)

    /**
     * @brief Whether we successfully set up push notifications with the server.
     */
    Q_PROPERTY(bool enablePushNotifications READ enablePushNotifications NOTIFY enablePushNotificationsChanged)

    /**
     * @brief True if this connection is a verified session.
     */
    Q_PROPERTY(bool isVerifiedSession READ isVerifiedSession NOTIFY ownSessionVerified)

public:
    /**
     * @brief Defines the status after an attempt to change the password on an account.
     */
    enum PasswordStatus {
        Success, /**< The password was successfully changed. */
        Wrong, /**< The current password entered was wrong. */
        Other, /**< An unknown problem occurred. */
    };
    Q_ENUM(PasswordStatus)

    NeoChatConnection(QObject *parent = nullptr);
    NeoChatConnection(const QUrl &server, QObject *parent = nullptr);

    Q_INVOKABLE void logout(bool serverSideLogout);
    Q_INVOKABLE QVariantList getSupportedRoomVersions() const;
    bool canCheckMutualRooms() const;
    bool canEraseData() const;

    /**
     * @brief Change the password for an account.
     *
     * The function emits a passwordStatus signal with a PasswordStatus value when
     * complete.
     *
     * @sa PasswordStatus, passwordStatus
     */
    Q_INVOKABLE void changePassword(const QString &currentPassword, const QString &newPassword);

    /**
     * @brief Change the avatar for an account.
     */
    Q_INVOKABLE bool setAvatar(const QUrl &avatarSource);

    [[nodiscard]] QString label() const;
    void setLabel(const QString &label);

    Q_INVOKABLE void deactivateAccount(const QString &password, bool erase);

    bool hasIdentityServer() const;

    /**
     * @brief The identity server URL.
     *
     * Empty if no identity server configured.
     */
    QUrl identityServer() const;

    QString identityServerUIString() const;

    /**
     * @brief Create new room for a group chat.
     */
    Q_INVOKABLE void createRoom(const QString &name, const QString &topic, const QString &parent = {}, bool setChildParent = false);

    /**
     * @brief Create new space.
     */
    Q_INVOKABLE void createSpace(const QString &name, const QString &topic, const QString &parent = {}, bool setChildParent = false);

    /**
     * @brief Send /forget to the server and delete room locally.
     *
     * @note This wraps around the Quotient::Connection::forgetRoom() to allow
     *       roomAboutToBeLeft() to be emitted.
     */
    Quotient::ForgetRoomJob *forgetRoom(const QString &id);

    /**
     * @brief Whether a direct chat with the user exists.
     */
    Q_INVOKABLE bool directChatExists(Quotient::User *user);

    /**
     * @brief Get the account data with \param type as a formatted JSON string.
     */
    Q_INVOKABLE QString accountDataJsonString(const QString &type) const;

    Q_INVOKABLE Quotient::KeyImport::Error exportMegolmSessions(const QString &passphrase, const QString &path);

    qsizetype directChatNotifications() const;
    bool directChatsHaveHighlightNotifications() const;
    qsizetype homeNotifications() const;
    bool homeHaveHighlightNotifications() const;

    int badgeNotificationCount() const;
    void refreshBadgeNotificationCount();

    bool globalUrlPreviewEnabled();

    /**
     * @brief Whether URL previews are enabled globally by default for all connections.
     */
    static void setGlobalUrlPreviewDefault(bool useByDefault);

    /**
     * @brief The current default PushRuleAction for keyword rules.
     */
    PushRuleAction::Action keywordPushRuleDefault() const;

    /**
     * @brief Set the current default PushRuleAction for keyword rules.
     */
    static void setKeywordPushRuleDefault(PushRuleAction::Action defaultAction);

    qsizetype directChatInvites() const;
    qsizetype roomInvites() const;

    // note: this is intentionally a copied QString because
    // the reference could be destroyed before the task is finished
    QCoro::Task<void> setupPushNotifications(QString endpoint);

    bool pushNotificationsAvailable() const;
    bool enablePushNotifications() const;

    LinkPreviewer *previewerForLink(const QUrl &link);

    /**
     * @return True if this connection is a verified session.
     */
    bool isVerifiedSession() const;

    Q_INVOKABLE void unlockSSSS(const QString &secret);

    /**
      * @brief Report a user.
      */
    Q_INVOKABLE void reportUser(const QString &userId, const QString &reason);

Q_SIGNALS:
    void globalUrlPreviewEnabledChanged();
    void labelChanged();
    void identityServerChanged();
    void directChatNotificationsChanged();
    void directChatsHaveHighlightNotificationsChanged();
    void homeNotificationsChanged();
    void homeHaveHighlightNotificationsChanged();
    void directChatInvitesChanged();
    void roomInvitesChanged();
    void passwordStatus(NeoChatConnection::PasswordStatus status);
    void userConsentRequired(QUrl url);
    void badgeNotificationCountChanged(int count);
    void canCheckMutualRoomsChanged();
    void canEraseDataChanged();
    void enablePushNotificationsChanged();

    /**
     * @brief Request a message be shown to the user of the given type.
     */
    void showMessage(MessageType::Type messageType, const QString &message);

    /**
     * @brief Request a error message be shown to the user.
     */
    void errorOccured(const QString &error);

    /**
     * @brief The given room ID is about to be forgotten.
     */
    void roomAboutToBeLeft(const QString &id);

    /**
     * @brief When the connection's own verification state changes.
     */
    void ownSessionVerified();

    void keyBackupUnlocked();
    void keyBackupError();

private:
    static bool m_globalUrlPreviewDefault;
    static PushRuleAction::Action m_defaultAction;

    void connectSignals();

    int m_badgeNotificationCount = 0;

    QCache<QUrl, LinkPreviewer> m_linkPreviewers;

    bool m_canCheckMutualRooms = false;
    bool m_canEraseData = false;
    bool m_pushNotificationsEnabled = false;
};
