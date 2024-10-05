// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QCache>
#include <QObject>
#include <QQmlEngine>

#include <QCoroTask>
#include <Quotient/connection.h>

#if Quotient_VERSION_MINOR > 8
#include <Quotient/keyimport.h>
#endif

#include "enums/messagetype.h"
#include "linkpreviewer.h"
#include "models/threepidmodel.h"

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
    Q_PROPERTY(QString deviceKey READ deviceKey CONSTANT)
    Q_PROPERTY(QString encryptionKey READ encryptionKey CONSTANT)

    /**
     * @brief The model with the account's 3PIDs.
     */
    Q_PROPERTY(ThreePIdModel *threePIdModel READ threePIdModel CONSTANT)

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
     * @brief Whether there is at least one invite to a direct chat.
     */
    Q_PROPERTY(bool directChatInvites READ directChatInvites NOTIFY directChatInvitesChanged)

    /**
     * @brief Whether NeoChat is currently able to connect to the server.
     */
    Q_PROPERTY(bool isOnline READ isOnline WRITE setIsOnline NOTIFY isOnlineChanged)

    /**
     * @brief Whether the server supports querying a user's mutual rooms.
     */
    Q_PROPERTY(bool canCheckMutualRooms READ canCheckMutualRooms NOTIFY canCheckMutualRoomsChanged)

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

    Q_INVOKABLE void deactivateAccount(const QString &password);

    ThreePIdModel *threePIdModel() const;

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
     * @brief Whether a direct chat with the user exists.
     */
    Q_INVOKABLE bool directChatExists(Quotient::User *user);

    /**
     * @brief Get the account data with \param type as a formatted JSON string.
     */
    Q_INVOKABLE QString accountDataJsonString(const QString &type) const;

#if Quotient_VERSION_MINOR > 8
    Q_INVOKABLE Quotient::KeyImport::Error exportMegolmSessions(const QString &passphrase, const QString &path);
#endif

    qsizetype directChatNotifications() const;
    bool directChatsHaveHighlightNotifications() const;
    qsizetype homeNotifications() const;
    bool homeHaveHighlightNotifications() const;

    int badgeNotificationCount() const;
    void refreshBadgeNotificationCount();

    bool directChatInvites() const;

    // note: this is intentionally a copied QString because
    // the reference could be destroyed before the task is finished
    QCoro::Task<void> setupPushNotifications(QString endpoint);

    QString deviceKey() const;
    QString encryptionKey() const;

    bool isOnline() const;

    LinkPreviewer *previewerForLink(const QUrl &link);

Q_SIGNALS:
    void labelChanged();
    void identityServerChanged();
    void directChatNotificationsChanged();
    void directChatsHaveHighlightNotificationsChanged();
    void homeNotificationsChanged();
    void homeHaveHighlightNotificationsChanged();
    void directChatInvitesChanged();
    void isOnlineChanged();
    void passwordStatus(NeoChatConnection::PasswordStatus status);
    void userConsentRequired(QUrl url);
    void badgeNotificationCountChanged(NeoChatConnection *connection, int count);
    void canCheckMutualRoomsChanged();

    /**
     * @brief Request a message be shown to the user of the given type.
     */
    void showMessage(MessageType::Type messageType, const QString &message);

    /**
     * @brief Request a error message be shown to the user.
     */
    void errorOccured(const QString &error);

private:
    bool m_isOnline = true;
    void setIsOnline(bool isOnline);

    ThreePIdModel *m_threePIdModel;

    void connectSignals();

    int m_badgeNotificationCount = 0;

    QCache<QUrl, LinkPreviewer> m_linkPreviewers;

    bool m_canCheckMutualRooms = false;
};
