// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>

#include <QCoroTask>
#include <Quotient/connection.h>

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
     * @brief The total number of notifications for all direct chats.
     */
    Q_PROPERTY(qsizetype directChatNotifications READ directChatNotifications NOTIFY directChatNotificationsChanged)

    /**
     * @brief Whether there is at least one invite to a direct chat.
     */
    Q_PROPERTY(bool directChatInvites READ directChatInvites NOTIFY directChatInvitesChanged)

    /**
     * @brief Whether NeoChat is currently able to connect to the server.
     */
    Q_PROPERTY(bool isOnline READ isOnline WRITE setIsOnline NOTIFY isOnlineChanged)

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
     * @brief Join a direct chat with the given user ID.
     *
     * If a direct chat with the user doesn't exist one is created and then joined.
     */
    Q_INVOKABLE void openOrCreateDirectChat(const QString &userId);

    /**
     * @brief Join a direct chat with the given user object.
     *
     * If a direct chat with the user doesn't exist one is created and then joined.
     */
    Q_INVOKABLE void openOrCreateDirectChat(Quotient::User *user);

    qsizetype directChatNotifications() const;
    bool directChatInvites() const;

    // note: this is intentionally a copied QString because
    // the reference could be destroyed before the task is finished
    QCoro::Task<void> setupPushNotifications(QString endpoint);

    QString deviceKey() const;
    QString encryptionKey() const;

    bool isOnline() const;

Q_SIGNALS:
    void labelChanged();
    void directChatNotificationsChanged();
    void directChatInvitesChanged();
    void isOnlineChanged();
    void passwordStatus(NeoChatConnection::PasswordStatus status);
    void userConsentRequired(QUrl url);

private:
    bool m_isOnline = true;
    void setIsOnline(bool isOnline);

    void connectSignals();
};
