// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QApplication>
#include <QMediaPlayer>
#include <QMenu>
#include <QObject>

#include <KAboutData>
class QKeySequences;
class QNetworkConfigurationManager;

#include "connection.h"
#include "csapi/list_public_rooms.h"
#include "room.h"
#include "settings.h"
#include "user.h"

class NeoChatRoom;
class QQuickWindow;

using namespace Quotient;

class Controller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int accountCount READ accountCount NOTIFY accountCountChanged)
    Q_PROPERTY(bool quitOnLastWindowClosed READ quitOnLastWindowClosed WRITE setQuitOnLastWindowClosed NOTIFY quitOnLastWindowClosedChanged)
    Q_PROPERTY(Connection *activeConnection READ activeConnection WRITE setActiveConnection NOTIFY activeConnectionChanged)
    Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged)
    Q_PROPERTY(KAboutData aboutData READ aboutData WRITE setAboutData NOTIFY aboutDataChanged)
    Q_PROPERTY(bool supportSystemTray READ supportSystemTray CONSTANT)
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY isOnlineChanged)

public:
    static Controller &instance();

    [[nodiscard]] QVector<Connection *> connections() const;

    void setActiveConnection(Connection *connection);
    [[nodiscard]] Connection *activeConnection() const;

    void addConnection(Connection *c);
    void dropConnection(Connection *c);

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

    bool saveAccessTokenToFile(const AccountSettings &account, const QByteArray &accessToken);
    bool saveAccessTokenToKeyChain(const AccountSettings &account, const QByteArray &accessToken);

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
private:
    explicit Controller(QObject *parent = nullptr);
    ~Controller() override;

    QVector<Connection *> m_connections;
    QPointer<Connection> m_connection;
    bool m_busy = false;

    static QByteArray loadAccessTokenFromFile(const AccountSettings &account);
    QByteArray loadAccessTokenFromKeyChain(const AccountSettings &account);

    void loadSettings();
    void saveSettings() const;

    KAboutData m_aboutData;

private Q_SLOTS:
    void invokeLogin();

Q_SIGNALS:
    void busyChanged();
    /// Error occurred because of user inputs
    void errorOccured(const QString &error);

    /// \brief Emitted when an action made the user join a room.
    ///
    /// Either when a new room was created, a direct chat was started
    /// or a group chat was joined. The UI will react to this signal
    /// and switch to the newly joined room.
    void roomJoined(const QString &roomName);

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
    void showWindow();
    void openRoom(NeoChatRoom *room);
    void userConsentRequired(QUrl url);
    void testConnectionResult(const QString &connection, bool usable);
    void isOnlineChanged(bool isOnline);

public Q_SLOTS:
    void logout(Quotient::Connection *conn, bool serverSideLogout);
    static void playAudio(const QUrl &localFile);
    void changeAvatar(Quotient::Connection *conn, const QUrl &localFile);
    static void markAllMessagesAsRead(Quotient::Connection *conn);
    void saveWindowGeometry(QQuickWindow *);

private:
    QNetworkConfigurationManager *m_mgr;
};

// TODO libQuotient 0.7: Drop
class NeochatChangePasswordJob : public BaseJob
{
public:
    explicit NeochatChangePasswordJob(const QString &newPassword, bool logoutDevices, const Omittable<QJsonObject> &auth = none);
};

class NeochatDeleteDeviceJob : public BaseJob
{
public:
    explicit NeochatDeleteDeviceJob(const QString &deviceId, const Omittable<QJsonObject> &auth = none);
};
