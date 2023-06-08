// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QQuickItem>

#include <KFormat>

#include <jobs/basejob.h>
#include <settings.h>

class NeoChatRoom;
class NeoChatUser;
class TrayIcon;
class QWindow;
class QQuickTextDocument;

namespace Quotient
{
class Connection;
class Room;
}

namespace QKeychain
{
class ReadPasswordJob;
}

/**
 * @class Controller
 *
 * A singleton class designed to help manage the application.
 *
 * There are also a bunch of helper functions that currently don't fit anywhere
 * else.
 */
class Controller : public QObject
{
    Q_OBJECT

    /**
     * @brief The number of logged in accounts.
     */
    Q_PROPERTY(int accountCount READ accountCount NOTIFY accountCountChanged)

    /**
     * @brief The current connection for the rest of NeoChat to use.
     */
    Q_PROPERTY(Quotient::Connection *activeConnection READ activeConnection WRITE setActiveConnection NOTIFY activeConnectionChanged)

    /**
     * @brief The row number in the accounts directory of the active connection.
     */
    Q_PROPERTY(int activeConnectionIndex READ activeConnectionIndex NOTIFY activeConnectionIndexChanged)

    /**
     * @brief The account label for the active account.
     *
     * Account labels are a concept specific to NeoChat, allowing accounts to be
     * labelled, e.g. for "Work", "Private", etc.
     *
     * Set to an empty string to remove the label.
     */
    Q_PROPERTY(QString activeAccountLabel READ activeAccountLabel WRITE setActiveAccountLabel NOTIFY activeAccountLabelChanged)

    /**
     * @brief Whether the OS NeoChat is running on supports sytem tray icons.
     */
    Q_PROPERTY(bool supportSystemTray READ supportSystemTray CONSTANT)

    /**
     * @brief Whether KWindowSystem specific features are available.
     */
    Q_PROPERTY(bool hasWindowSystem READ hasWindowSystem CONSTANT)

    /**
     * @brief Whether NeoChat is currently able to connect to the server.
     */
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY isOnlineChanged)

    /**
     * @brief Whether the ecryption support has been enabled.
     */
    Q_PROPERTY(bool encryptionSupported READ encryptionSupported CONSTANT)

    /**
     * @brief The current minor version number of libQuotient being used.
     *
     * This is the only way to gate NeoChat features by libQuotient version in QML.
     *
     * @note No major version because libQuotient doesn't have any; All are 0.x.
     */
    Q_PROPERTY(int quotientMinorVersion READ quotientMinorVersion CONSTANT)

    /**
     * @brief Whether NeoChat is running as a flatpak.
     *
     * This is the only way to gate NeoChat features in flatpaks in QML.
     */
    Q_PROPERTY(bool isFlatpak READ isFlatpak CONSTANT)

public:
    /**
     * @brief Defines the status after an attempt to change the password on an account.
     */
    enum PasswordStatus {
        Success, /**< The password was successfully changed. */
        Wrong, /**< The current password entered was wrong. */
        Other, /**< An unknown problem occurred. */
    };
    Q_ENUM(PasswordStatus);

    static Controller &instance();

    [[nodiscard]] int accountCount() const;

    void setActiveConnection(Quotient::Connection *connection);
    [[nodiscard]] Quotient::Connection *activeConnection() const;

    /**
     * @brief Add a new connection to the account registry.
     */
    void addConnection(Quotient::Connection *c);

    /**
     * @brief Drop a connection from the account registry.
     */
    void dropConnection(Quotient::Connection *c);

    int activeConnectionIndex() const;

    [[nodiscard]] QString activeAccountLabel() const;
    void setActiveAccountLabel(const QString &label);

    /**
     * @brief Save an access token to the keychain for the given account.
     */
    bool saveAccessTokenToKeyChain(const Quotient::AccountSettings &account, const QByteArray &accessToken);

    /**
     * @brief Change the password for an account.
     *
     * The function emits a passwordStatus signal with a PasswordStatus value when
     * complete.
     *
     * @sa PasswordStatus, passwordStatus
     */
    Q_INVOKABLE void changePassword(Quotient::Connection *connection, const QString &currentPassword, const QString &newPassword);

    /**
     * @brief Change the avatar for an account.
     */
    Q_INVOKABLE bool setAvatar(Quotient::Connection *connection, const QUrl &avatarSource);

    /**
     * @brief Create new room for a group chat.
     */
    Q_INVOKABLE void createRoom(const QString &name, const QString &topic);

    /**
     * @brief Create new space.
     */
    Q_INVOKABLE void createSpace(const QString &name, const QString &topic);

    /**
     * @brief Join a room.
     */
    Q_INVOKABLE void joinRoom(const QString &alias);

    /**
     * @brief Join a direct chat with the given user.
     *
     * If a direct chat with the user doesn't exist one is created and then joined.
     */
    Q_INVOKABLE void openOrCreateDirectChat(NeoChatUser *user);

    [[nodiscard]] bool supportSystemTray() const;

    /**
     * @brief Set the background blur status of the given item.
     */
    Q_INVOKABLE void setBlur(QQuickItem *item, bool blur);

    bool isOnline() const;

    bool encryptionSupported() const;

    /**
     * @brief Sets the QNetworkProxy for the application.
     *
     * @sa QNetworkProxy::setApplicationProxy
     */
    Q_INVOKABLE void setApplicationProxy();

    int quotientMinorVersion() const;

    bool isFlatpak() const;

    /**
     * @brief Return a string for the input timestamp.
     *
     * The output format depends on the KFormat::DurationFormatOptions chosen.
     *
     * @sa KFormat::DurationFormatOptions
     */
    Q_INVOKABLE QString formatDuration(quint64 msecs, KFormat::DurationFormatOptions options = KFormat::DefaultDuration) const;

    /**
     * @brief Return a human readable string for a given input number of bytes.
     */
    Q_INVOKABLE QString formatByteSize(double size, int precision = 1) const;

    /**
     * @brief Force a QQuickTextDocument to refresh when images are loaded.
     *
     * HACK: This is a workaround for QTBUG 93281.
     */
    Q_INVOKABLE void forceRefreshTextDocument(QQuickTextDocument *textDocument, QQuickItem *item);

    Q_INVOKABLE QVariantList getSupportedRoomVersions(Quotient::Connection *connection);

private:
    explicit Controller(QObject *parent = nullptr);

    QPointer<Quotient::Connection> m_connection;
    TrayIcon *m_trayIcon = nullptr;

    QKeychain::ReadPasswordJob *loadAccessTokenFromKeyChain(const Quotient::AccountSettings &account);

    void loadSettings();
    void saveSettings() const;
    bool m_isOnline = true;
    QMap<Quotient::Room *, int> m_notificationCounts;

    bool hasWindowSystem() const;

private Q_SLOTS:
    void invokeLogin();
    void showWindow();
    void setQuitOnLastWindowClosed();

Q_SIGNALS:
    /// Error occurred because of user inputs
    void errorOccured(const QString &error);

    /// Error occurred because of server or bug in NeoChat
    void globalErrorOccured(QString error, QString detail);
    void syncDone();
    void connectionAdded(Quotient::Connection *_t1);
    void connectionDropped(Quotient::Connection *_t1);
    void accountCountChanged();
    void initiated();
    void notificationClicked(const QString &_t1, const QString &_t2);
    void quitOnLastWindowClosedChanged();
    void unreadCountChanged();
    void activeConnectionChanged();
    void passwordStatus(Controller::PasswordStatus _t1);
    void userConsentRequired(QUrl url);
    void testConnectionResult(const QString &connection, bool usable);
    void isOnlineChanged(bool isOnline);
    void keyVerificationRequest(int timeLeft, Quotient::Connection *connection, const QString &transactionId, const QString &deviceId);
    void keyVerificationStart();
    void keyVerificationAccept(const QString &commitment);
    void keyVerificationKey(const QString &sas);
    void activeConnectionIndexChanged();
    void roomAdded(NeoChatRoom *room);
    void activeAccountLabelChanged();

public Q_SLOTS:
    void logout(Quotient::Connection *conn, bool serverSideLogout);
    void changeAvatar(Quotient::Connection *conn, const QUrl &localFile);
    static void markAllMessagesAsRead(Quotient::Connection *conn);
    void saveWindowGeometry();
};

// TODO libQuotient 0.7: Drop
class NeochatChangePasswordJob : public Quotient::BaseJob
{
public:
    explicit NeochatChangePasswordJob(const QString &newPassword, bool logoutDevices, const Quotient::Omittable<QJsonObject> &auth = Quotient::none);
};

class NeochatDeleteDeviceJob : public Quotient::BaseJob
{
public:
    explicit NeochatDeleteDeviceJob(const QString &deviceId, const Quotient::Omittable<QJsonObject> &auth = Quotient::none);
};
