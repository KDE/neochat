// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include <Quotient/accountregistry.h>

#include "neochatconnection.h"

class AccountManager : public QObject
{
    Q_OBJECT

public:
    explicit AccountManager(bool testMode = false, QObject *parent = nullptr);

    Quotient::AccountRegistry *accounts();

    /**
     * @brief Load the accounts saved in the app cache.
     *
     * This should be called on app startup to retrieve accounts logged in, in a
     * previous sessions.
     */
    void loadAccountsFromCache();

    /**
     * @brief The accounts currently being loaded from cache.
     */
    QStringList accountsLoading() const;

    /**
     * @brief Get the primary connection being displayed in the rest of the app.
     */
    NeoChatConnection *activeConnection() const;

    /**
     * @brief Set the primary connection being displayed in the rest of the app.
     */
    void setActiveConnection(NeoChatConnection *connection);

    /**
     * @brief Add a new connection to the account registry.
     */
    void addConnection(NeoChatConnection *connection);

    /**
     * @brief Drop a connection from the account registry.
     */
    Q_INVOKABLE void dropConnection(const QString &userId);

    /**
     * @brief Drop a connection from the account registry.
     */
    void dropConnection(NeoChatConnection *connection);

Q_SIGNALS:
    /**
     * @brief Request a error message be shown to the user.
     */
    void errorOccured(const QString &error);

    /**
     * @brief The list of accounts loading the access token from keychain has changed.
     */
    void accountsLoadingChanged();

    /**
     * @brief The list of connection loading has changed.
     */
    void connectionLoadingChanged();

    /**
     * @brief The given connection has been added.
     */
    void connectionAdded(NeoChatConnection *connection);

    /**
     * @brief The given connection has been dropped.
     */
    void connectionDropped(NeoChatConnection *connection);

    /**
     * @brief The primary connection being displayed in the rest of the app has changed.
     */
    void activeConnectionChanged(NeoChatConnection *oldConnection, NeoChatConnection *newConnection);

private:
    QPointer<Quotient::AccountRegistry> m_accountRegistry;
    QStringList m_accountsLoading;
    QMap<QString, QPointer<NeoChatConnection>> m_connectionsLoading;

    void saveAccessTokenToKeyChain(NeoChatConnection *connection);
    QKeychain::ReadPasswordJob *loadAccessTokenFromKeyChain(const QString &userId);

    bool dropAccountLoading(const QString &userId);
    bool dropConnectionLoading(NeoChatConnection *connection);
    bool dropRegistry(NeoChatConnection *connection);

    QPointer<NeoChatConnection> m_activeConnection;
};
