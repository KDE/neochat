// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QQmlEngine>

#include "neochatconnection.h"
#include <Quotient/accountregistry.h>

class TrayIcon;
class QQuickTextDocument;

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
     * @brief Whether NeoChat is running as a flatpak.
     *
     * This is the only way to gate NeoChat features in flatpaks in QML.
     */
    Q_PROPERTY(bool isFlatpak READ isFlatpak CONSTANT)

    Q_PROPERTY(QStringList accountsLoading MEMBER m_accountsLoading NOTIFY accountsLoadingChanged)

    Q_PROPERTY(bool ssssSupported READ ssssSupported CONSTANT)
    Q_PROPERTY(bool csSupported READ csSupported CONSTANT)

public:
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
    bool saveAccessTokenToKeyChain(const QString &userId, const QByteArray &accessToken);

    [[nodiscard]] bool supportSystemTray() const;

    bool isFlatpak() const;

    /**
     * @brief Start listening for notifications in dbus-activated mode.
     * These notifications will quit the application when closed.
     */
    static void listenForNotifications();

    Q_INVOKABLE QString loadFileContent(const QString &path) const;

    Quotient::AccountRegistry &accounts();

    static void setTestMode(bool testMode);

    Q_INVOKABLE void removeConnection(const QString &userId);

    bool ssssSupported() const;
    bool csSupported() const;

private:
    explicit Controller(QObject *parent = nullptr);

    QPointer<NeoChatConnection> m_connection;
    TrayIcon *m_trayIcon = nullptr;

    QKeychain::ReadPasswordJob *loadAccessTokenFromKeyChain(const QString &account);

    void loadSettings();
    void saveSettings() const;

    Quotient::AccountRegistry m_accountRegistry;
    QStringList m_accountsLoading;
    QMap<QString, QPointer<NeoChatConnection>> m_connectionsLoading;
    QString m_endpoint;

private Q_SLOTS:
    void invokeLogin();
    void setQuitOnLastWindowClosed();
    void updateBadgeNotificationCount(NeoChatConnection *connection, int count);

Q_SIGNALS:
    void errorOccured(const QString &error, const QString &detail);
    void connectionAdded(NeoChatConnection *connection);
    void connectionDropped(NeoChatConnection *connection);
    void activeConnectionChanged(NeoChatConnection *connection);
    void accountsLoadingChanged();
};
