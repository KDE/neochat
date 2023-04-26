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

class Controller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int accountCount READ accountCount NOTIFY accountCountChanged)
    Q_PROPERTY(Quotient::Connection *activeConnection READ activeConnection WRITE setActiveConnection NOTIFY activeConnectionChanged)
    Q_PROPERTY(bool supportSystemTray READ supportSystemTray CONSTANT)
    Q_PROPERTY(bool hasWindowSystem READ hasWindowSystem CONSTANT)
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY isOnlineChanged)
    Q_PROPERTY(bool encryptionSupported READ encryptionSupported CONSTANT)
    Q_PROPERTY(int activeConnectionIndex READ activeConnectionIndex NOTIFY activeConnectionIndexChanged)
    Q_PROPERTY(int quotientMinorVersion READ quotientMinorVersion CONSTANT)
    Q_PROPERTY(bool isFlatpak READ isFlatpak CONSTANT)
    Q_PROPERTY(QString activeAccountLabel READ activeAccountLabel WRITE setActiveAccountLabel NOTIFY activeAccountLabelChanged)

public:
    static Controller &instance();

    void setActiveConnection(Quotient::Connection *connection);
    [[nodiscard]] Quotient::Connection *activeConnection() const;

    void addConnection(Quotient::Connection *c);
    void dropConnection(Quotient::Connection *c);

    Q_INVOKABLE void changePassword(Quotient::Connection *connection, const QString &currentPassword, const QString &newPassword);

    Q_INVOKABLE bool setAvatar(Quotient::Connection *connection, const QUrl &avatarSource);

    [[nodiscard]] int accountCount() const;

    [[nodiscard]] bool supportSystemTray() const;

    bool saveAccessTokenToKeyChain(const Quotient::AccountSettings &account, const QByteArray &accessToken);

    int activeConnectionIndex() const;

    enum PasswordStatus {
        Success,
        Wrong,
        Other,
    };
    Q_ENUM(PasswordStatus);

    /// \brief Create new room for a group chat.
    Q_INVOKABLE void createRoom(const QString &name, const QString &topic);

    /// \brief Join a room.
    Q_INVOKABLE void joinRoom(const QString &alias);

    bool isOnline() const;

    Q_INVOKABLE QString formatDuration(quint64 msecs, KFormat::DurationFormatOptions options = KFormat::DefaultDuration) const;
    Q_INVOKABLE QString formatByteSize(double size, int precision = 1) const;

    Q_INVOKABLE void openOrCreateDirectChat(NeoChatUser *user);

    Q_INVOKABLE void setBlur(QQuickItem *item, bool blur);
    Q_INVOKABLE QString plainText(QQuickTextDocument *document) const;
    bool encryptionSupported() const;

    Q_INVOKABLE void forceRefreshTextDocument(QQuickTextDocument *textDocument, QQuickItem *item);

    Q_INVOKABLE void setApplicationProxy();

    int quotientMinorVersion() const;
    bool isFlatpak() const;

    /**
     * @brief The label for this account.
     *
     * Account labels are a concept Specific to NeoChat, allowing accounts to be labelled, e.g. for "Work", "Private", etc.
     * @return The label, if it exists, otherwise an empty string
     */
    [[nodiscard]] QString activeAccountLabel() const;

    /**
     * @brief Set the label for this account.
     *
     * Set to an empty string to remove the label
     * @sa Controller::activeAccountLabel
     * @param label The label to use, or an empty string
     */
    void setActiveAccountLabel(const QString &label);

private:
    explicit Controller(QObject *parent = nullptr);

    QPointer<Quotient::Connection> m_connection;
    bool m_busy = false;
    TrayIcon *m_trayIcon = nullptr;

    QKeychain::ReadPasswordJob *loadAccessTokenFromKeyChain(const Quotient::AccountSettings &account);

    void loadSettings();
    void saveSettings() const;
    bool m_isOnline = true;
    QMap<Quotient::Room *, int> m_notificationCounts;

    bool hasWindowSystem() const;
#ifdef QUOTIENT_07
    void handleNotifications(QPointer<Quotient::Connection> connection);
#endif

private Q_SLOTS:
    void invokeLogin();
    void showWindow();
    void setQuitOnLastWindowClosed();

Q_SIGNALS:
    void busyChanged();
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
