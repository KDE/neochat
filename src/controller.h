/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QApplication>
#include <QMediaPlayer>
#include <QMenu>
#include <QObject>

#include <KAboutData>
class QKeySequences;

#include "connection.h"
#include "csapi/list_public_rooms.h"
#include "room.h"
#include "settings.h"
#include "user.h"

class NeoChatRoom;

using namespace Quotient;

class Controller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int accountCount READ accountCount NOTIFY connectionAdded NOTIFY connectionDropped)
    Q_PROPERTY(bool quitOnLastWindowClosed READ quitOnLastWindowClosed WRITE setQuitOnLastWindowClosed NOTIFY quitOnLastWindowClosedChanged)
    Q_PROPERTY(Connection *activeConnection READ activeConnection WRITE setActiveConnection NOTIFY activeConnectionChanged)
    Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged)
    Q_PROPERTY(KAboutData aboutData READ aboutData WRITE setAboutData NOTIFY aboutDataChanged)

    /// Get the list of shortcuts activating the preferences page
    Q_PROPERTY(QList<QKeySequence> preferencesShortcuts READ preferencesShortcuts CONSTANT)

public:
    static Controller &instance();

    [[nodiscard]] QVector<Connection *> connections() const;

    void setActiveConnection(Connection *connection);
    [[nodiscard]] Connection *activeConnection() const;

    void addConnection(Connection *c);
    void dropConnection(Connection *c);

    Q_INVOKABLE void loginWithCredentials(const QString &, const QString &, const QString &, QString);
    Q_INVOKABLE void loginWithAccessToken(const QString &, const QString &, const QString &, const QString &);

    Q_INVOKABLE void changePassword(Quotient::Connection *connection, const QString &currentPassword, const QString &newPassword);

    [[nodiscard]] int accountCount() const;

    [[nodiscard]] QList<QKeySequence> preferencesShortcuts() const;

    [[nodiscard]] static bool quitOnLastWindowClosed();
    void setQuitOnLastWindowClosed(bool value);

    [[nodiscard]] bool busy() const;
    void setBusy(bool busy);

    void setAboutData(const KAboutData &aboutData);
    [[nodiscard]] KAboutData aboutData() const;

    enum PasswordStatus {
        Success,
        Wrong,
        Other,
    };
    Q_ENUM(PasswordStatus);

private:
    explicit Controller(QObject *parent = nullptr);
    ~Controller() override;

    QVector<Connection *> m_connections;
    QPointer<Connection> m_connection;
    bool m_busy = false;

    static QByteArray loadAccessTokenFromFile(const AccountSettings &account);
    QByteArray loadAccessTokenFromKeyChain(const AccountSettings &account);

    bool saveAccessTokenToFile(const AccountSettings &account, const QByteArray &accessToken);
    bool saveAccessTokenToKeyChain(const AccountSettings &account, const QByteArray &accessToken);
    void loadSettings();
    void saveSettings() const;

    KAboutData m_aboutData;

private Q_SLOTS:
    void invokeLogin();

Q_SIGNALS:
    void busyChanged();
    /// Error occured because of user inputs
    void errorOccured(QString error, QString detail);

    /// Error occured because of server or bug in NeoChat
    void globalErrorOccured(QString error, QString detail);
    void syncDone();
    void connectionAdded(Quotient::Connection *_t1);
    void connectionDropped(Quotient::Connection *_t1);
    void initiated();
    void notificationClicked(const QString &_t1, const QString &_t2);
    void quitOnLastWindowClosedChanged();
    void unreadCountChanged();
    void activeConnectionChanged();
    void aboutDataChanged();
    void passwordStatus(Controller::PasswordStatus _t1);
    void showWindow();
    void openRoom(NeoChatRoom *room);

public Q_SLOTS:
    void logout(Quotient::Connection *conn, bool serverSideLogout);
    void joinRoom(Quotient::Connection *c, const QString &alias);
    void createRoom(Quotient::Connection *c, const QString &name, const QString &topic);
    void createDirectChat(Quotient::Connection *c, const QString &userID);
    static void playAudio(const QUrl &localFile);
    void changeAvatar(Quotient::Connection *conn, const QUrl &localFile);
    static void markAllMessagesAsRead(Quotient::Connection *conn);
};

// TODO libQuotient 0.7: Drop
class NeochatChangePasswordJob : public BaseJob
{
public:
    explicit NeochatChangePasswordJob(const QString &newPassword, bool logoutDevices, const Omittable<QJsonObject> &auth = none);
};

class NeochatDeleteDeviceJob : public BaseJob {
public:
    explicit NeochatDeleteDeviceJob(const QString& deviceId, const Omittable<QJsonObject> &auth = none);
};

#endif // CONTROLLER_H
