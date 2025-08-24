// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QQmlEngine>

#include "accountmanager.h"
#include "neochatconnection.h"
#include "notificationsmanager.h"
#include <Quotient/accountregistry.h>

class TrayIcon;

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

    Q_PROPERTY(QStringList accountsLoading READ accountsLoading NOTIFY accountsLoadingChanged)

public:
    static Controller &instance();
    static Controller *create(QQmlEngine *engine, QJSEngine *)
    {
        engine->setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    void setAccountManager(AccountManager *manager);

    [[nodiscard]] NeoChatConnection *activeConnection() const;
    void setActiveConnection(NeoChatConnection *connection);

    QStringList accountsLoading() const;

    [[nodiscard]] bool supportSystemTray() const;

    bool isFlatpak() const;

    /**
     * @brief Start listening for notifications in dbus-activated mode.
     * These notifications will quit the application when closed.
     */
    static void listenForNotifications();

    /**
     * @brief Clear an existing invite notification for the given room.
     *
     * Nothing happens if the given room doesn't have an invite notification.
     */
    Q_INVOKABLE void clearInvitationNotification(const QString &roomId);

    Q_INVOKABLE QString loadFileContent(const QString &path) const;

    Quotient::AccountRegistry *accounts();

    Q_INVOKABLE void removeConnection(const QString &userId);

    /**
     * @brief Revert all configuration values to their default.
     *
     * The parameters along with their defaults are specified in the config file
     * neochatconfig.kcfg.
     */
    Q_INVOKABLE void revertToDefaultConfig();

    Q_INVOKABLE bool isImageShown(const QString &eventId);
    Q_INVOKABLE void markImageShown(const QString &eventId);
    Q_INVOKABLE void markImageHidden(const QString &eventId);

private:
    explicit Controller(QObject *parent = nullptr);

    QPointer<AccountManager> m_accountManager;
    void initConnection(NeoChatConnection *connection);
    void teardownConnection(NeoChatConnection *connection);
    void initActiveConnection(NeoChatConnection *oldConnection, NeoChatConnection *newConnection);

    QPointer<NeoChatConnection> m_connection;
#ifndef Q_OS_ANDROID
    QPointer<TrayIcon> m_trayIcon;
#endif
    QString m_endpoint;
    QStringList m_shownImages;

    NotificationsManager m_notificationsManager;

private Q_SLOTS:
    void setQuitOnLastWindowClosed();
    void updateBadgeNotificationCount(int count);

Q_SIGNALS:
    /**
     * @brief Request a error message be shown to the user.
     */
    void errorOccured(const QString &error);
    void connectionAdded(NeoChatConnection *connection);
    void connectionDropped(NeoChatConnection *connection);
    void activeConnectionChanged(NeoChatConnection *connection);
    void accountsLoadingChanged();

    void globalUrlPreviewDefaultChanged();
};
