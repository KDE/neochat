// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QQuickItem>

#include <KAboutData>
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
    Q_PROPERTY(bool quitOnLastWindowClosed READ quitOnLastWindowClosed WRITE setQuitOnLastWindowClosed NOTIFY quitOnLastWindowClosedChanged)
    Q_PROPERTY(Quotient::Connection *activeConnection READ activeConnection WRITE setActiveConnection NOTIFY activeConnectionChanged)
    Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged)
    Q_PROPERTY(KAboutData aboutData READ aboutData WRITE setAboutData NOTIFY aboutDataChanged)
    Q_PROPERTY(bool supportSystemTray READ supportSystemTray CONSTANT)
    Q_PROPERTY(bool hasWindowSystem READ hasWindowSystem CONSTANT)
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY isOnlineChanged)
    Q_PROPERTY(bool encryptionSupported READ encryptionSupported CONSTANT)

public:
    static Controller &instance();

    void setActiveConnection(Quotient::Connection *connection);
    [[nodiscard]] Quotient::Connection *activeConnection() const;

    void addConnection(Quotient::Connection *c);
    void dropConnection(Quotient::Connection *c);

    Q_INVOKABLE void loginWithAccessToken(const QString &, const QString &, const QString &, const QString &);

    Q_INVOKABLE void changePassword(Quotient::Connection *connection, const QString &currentPassword, const QString &newPassword);

    Q_INVOKABLE bool setAvatar(Quotient::Connection *connection, const QUrl &avatarSource);

    [[nodiscard]] int accountCount() const;

    [[nodiscard]] static bool quitOnLastWindowClosed();
    void setQuitOnLastWindowClosed(bool value);

    [[nodiscard]] bool busy() const;
    void setBusy(bool busy);

    void setAboutData(const KAboutData &aboutData);
    [[nodiscard]] KAboutData aboutData() const;

    [[nodiscard]] bool supportSystemTray() const;

    bool saveAccessTokenToFile(const Quotient::AccountSettings &account, const QByteArray &accessToken);
    bool saveAccessTokenToKeyChain(const Quotient::AccountSettings &account, const QByteArray &accessToken);

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

private:
    explicit Controller(QObject *parent = nullptr);

    QPointer<Quotient::Connection> m_connection;
    bool m_busy = false;
    TrayIcon *m_trayIcon = nullptr;

    static QByteArray loadAccessTokenFromFile(const Quotient::AccountSettings &account);
    QKeychain::ReadPasswordJob *loadAccessTokenFromKeyChain(const Quotient::AccountSettings &account);

    void loadSettings();
    void saveSettings() const;
    bool m_isOnline = true;
    QMap<Quotient::Room *, int> m_notificationCounts;

    KAboutData m_aboutData;
    bool hasWindowSystem() const;
#ifdef QUOTIENT_07
    void handleNotifications();
#endif

private Q_SLOTS:
    void invokeLogin();
    void showWindow();

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
    void aboutDataChanged();
    void passwordStatus(Controller::PasswordStatus _t1);
    void userConsentRequired(QUrl url);
    void testConnectionResult(const QString &connection, bool usable);
    void isOnlineChanged(bool isOnline);
    void keyVerificationRequest(int timeLeft, Quotient::Connection *connection, const QString &transactionId, const QString &deviceId);
    void keyVerificationStart();
    void keyVerificationAccept(const QString &commitment);
    void keyVerificationKey(const QString &sas);

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
