// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>

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
     * @brief Whether NeoChat is currently able to connect to the server.
     */
    Q_PROPERTY(bool isOnline READ isOnline WRITE setIsOnline NOTIFY isOnlineChanged)

public:
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
     * @brief Join a direct chat with the given user.
     *
     * If a direct chat with the user doesn't exist one is created and then joined.
     */
    Q_INVOKABLE void openOrCreateDirectChat(Quotient::User *user);

    QString deviceKey() const;
    QString encryptionKey() const;

    bool isOnline() const;

Q_SIGNALS:
    void labelChanged();
    void isOnlineChanged();

private:
    bool m_isOnline = true;
    void setIsOnline(bool isOnline);
};
