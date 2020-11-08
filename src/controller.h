/**
 * SPDX-FileCopyrightText: Black Hat <bhat@encom.eu.org>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-only
 */
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QApplication>
#include <QMediaPlayer>
#include <QMenu>
#include <QNetworkConfigurationManager>
#include <QObject>
#include <QSystemTrayIcon>

#include <KAboutData>

#include "connection.h"
#include "csapi/list_public_rooms.h"
#include "room.h"
#include "settings.h"
#include "user.h"

using namespace Quotient;

class Controller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int accountCount READ accountCount NOTIFY connectionAdded NOTIFY connectionDropped)
    Q_PROPERTY(bool quitOnLastWindowClosed READ quitOnLastWindowClosed WRITE setQuitOnLastWindowClosed NOTIFY quitOnLastWindowClosedChanged)
    Q_PROPERTY(Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY isOnlineChanged)
    Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged)
    Q_PROPERTY(KAboutData aboutData READ aboutData WRITE setAboutData NOTIFY aboutDataChanged)

public:
    static Controller &instance();

    QVector<Connection *> connections() const;

    void setConnection(Connection *conn);
    Connection *connection() const;

    void addConnection(Connection *c);
    void dropConnection(Connection *c);

    Q_INVOKABLE void loginWithCredentials(QString, QString, QString, QString);
    Q_INVOKABLE void loginWithAccessToken(QString, QString, QString, QString);

    Q_INVOKABLE void changePassword(Connection *connection, const QString &currentPassword, const QString &newPassword);

    int accountCount() const;

    bool isOnline() const;

    bool quitOnLastWindowClosed() const;
    void setQuitOnLastWindowClosed(bool value);

    bool busy() const;
    void setBusy(bool busy);

    void setAboutData(KAboutData aboutData);
    KAboutData aboutData() const;

    enum PasswordStatus {
        Success,
        Wrong,
        Other,
    };
    Q_ENUM(PasswordStatus);

private:
    explicit Controller(QObject *parent = nullptr);
    ~Controller();

    QVector<Connection *> m_connections;
    QPointer<Connection> m_connection;
    QNetworkConfigurationManager m_ncm;
    bool m_busy = false;

    QByteArray loadAccessTokenFromFile(const AccountSettings &account);
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
    void errorOccured(QString error, QString detail);
    void syncDone();
    void connectionAdded(Quotient::Connection *conn);
    void connectionDropped(Quotient::Connection *conn);
    void initiated();
    void notificationClicked(const QString roomId, const QString eventId);
    void quitOnLastWindowClosedChanged();
    void unreadCountChanged();
    void connectionChanged();
    void isOnlineChanged();
    void aboutDataChanged();
    void passwordStatus(PasswordStatus status);

public Q_SLOTS:
    void logout(Quotient::Connection *conn);
    void joinRoom(Quotient::Connection *c, const QString &alias);
    void createRoom(Quotient::Connection *c, const QString &name, const QString &topic);
    void createDirectChat(Quotient::Connection *c, const QString &userID);
    void playAudio(QUrl localFile);
    void changeAvatar(Quotient::Connection *conn, QUrl localFile);
    void markAllMessagesAsRead(Quotient::Connection *conn);
};

// TODO libQuotient 0.7: Drop
class NeochatChangePasswordJob : public BaseJob
{
public:
    explicit NeochatChangePasswordJob(const QString &newPassword, bool logoutDevices, const Omittable<QJsonObject> &auth = none);
};

#endif // CONTROLLER_H
