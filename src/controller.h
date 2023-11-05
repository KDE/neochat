// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>

#include "neochatconnection.h"
#include <Quotient/accountregistry.h>
#include <Quotient/jobs/basejob.h>
#include <Quotient/settings.h>

class NeoChatRoom;
class TrayIcon;
class QQuickTextDocument;

namespace Quotient
{
class Room;
class User;
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
    QML_ELEMENT
    QML_SINGLETON

    /**
     * @brief The current connection for the rest of NeoChat to use.
     */
    Q_PROPERTY(NeoChatConnection *activeConnection READ activeConnection WRITE setActiveConnection NOTIFY activeConnectionChanged)

    /**
     * @brief Whether the OS NeoChat is running on supports sytem tray icons.
     */
    Q_PROPERTY(bool supportSystemTray READ supportSystemTray CONSTANT)

    /**
     * @brief Whether NeoChat is currently able to connect to the server.
     */
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY isOnlineChanged)

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
    Q_ENUM(PasswordStatus)

    static Controller &instance();
    static Controller *create(QQmlEngine *engine, QJSEngine *)
    {
        engine->setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    void setActiveConnection(NeoChatConnection *connection);
    [[nodiscard]] NeoChatConnection *activeConnection() const;

    /**
     * @brief Add a new connection to the account registry.
     */
    void addConnection(NeoChatConnection *c);

    /**
     * @brief Drop a connection from the account registry.
     */
    void dropConnection(NeoChatConnection *c);

    /**
     * @brief Save an access token to the keychain for the given account.
     */
    bool saveAccessTokenToKeyChain(const Quotient::AccountSettings &account, const QByteArray &accessToken);

    /**
     * @brief Join a room.
     */
    Q_INVOKABLE void joinRoom(const QString &alias);

    [[nodiscard]] bool supportSystemTray() const;

    bool isOnline() const;

    /**
     * @brief Sets the QNetworkProxy for the application.
     *
     * @sa QNetworkProxy::setApplicationProxy
     */
    Q_INVOKABLE void setApplicationProxy();

    bool isFlatpak() const;

    /**
     * @brief Force a QQuickTextDocument to refresh when images are loaded.
     *
     * HACK: This is a workaround for QTBUG 93281.
     */
    Q_INVOKABLE void forceRefreshTextDocument(QQuickTextDocument *textDocument, QQuickItem *item);

    Quotient::AccountRegistry &accounts();

private:
    explicit Controller(QObject *parent = nullptr);

    QPointer<NeoChatConnection> m_connection;
    TrayIcon *m_trayIcon = nullptr;

    QKeychain::ReadPasswordJob *loadAccessTokenFromKeyChain(const Quotient::AccountSettings &account);

    void loadSettings();
    void saveSettings() const;
    bool m_isOnline = true;
    QMap<Quotient::Room *, int> m_notificationCounts;

    Quotient::AccountRegistry m_accountRegistry;

private Q_SLOTS:
    void invokeLogin();
    void toggleWindow();
    void setQuitOnLastWindowClosed();

Q_SIGNALS:
    /// Error occurred because of user inputs
    void errorOccured(const QString &error);

    /// Error occurred because of server or bug in NeoChat
    void globalErrorOccured(QString error, QString detail);
    void syncDone();
    void connectionAdded(NeoChatConnection *connection);
    void connectionDropped(NeoChatConnection *connection);
    void initiated();
    void quitOnLastWindowClosedChanged();
    void unreadCountChanged();
    void activeConnectionChanged();
    void passwordStatus(Controller::PasswordStatus status);
    void userConsentRequired(QUrl url);
    void isOnlineChanged(bool isOnline);

public Q_SLOTS:
    void saveWindowGeometry();
};
