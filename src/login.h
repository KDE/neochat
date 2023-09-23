// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QUrl>

class NeoChatConnection;

/**
 * @class LoginHelper
 *
 * A helper class for logging into a Matrix account.
 */
class LoginHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    /**
     * @brief Whether the home server for the account is reachable.
     */
    Q_PROPERTY(bool homeserverReachable READ homeserverReachable NOTIFY homeserverReachableChanged)

    /**
     * @brief Whether the connection to the home server is being tested.
     *
     * True if NeoChat is trying to resolve the home server, false if not started
     * or complete.
     */
    Q_PROPERTY(bool testing READ testing NOTIFY testingChanged)

    /**
     * @brief The Matrix ID of the account that is being logged into.
     */
    Q_PROPERTY(QString matrixId READ matrixId WRITE setMatrixId NOTIFY matrixIdChanged)

    /**
     * @brief The password entered by the user to login to the account.
     */
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)

    /**
     * @brief The device name to assign to this session.
     */
    Q_PROPERTY(QString deviceName READ deviceName WRITE setDeviceName NOTIFY deviceNameChanged)

    /**
     * @brief Whether the home server of the account supports single sign on login.
     */
    Q_PROPERTY(bool supportsSso READ supportsSso NOTIFY loginFlowsChanged STORED false)

    /**
     * @brief Whether the home server of the account supports password login.
     */
    Q_PROPERTY(bool supportsPassword READ supportsPassword NOTIFY loginFlowsChanged STORED false)

    /**
     * @brief The URL for the single sign on session.
     */
    Q_PROPERTY(QUrl ssoUrl READ ssoUrl NOTIFY ssoUrlChanged)

    /**
     * @brief Whether login process is ongoing.
     */
    Q_PROPERTY(bool isLoggingIn READ isLoggingIn NOTIFY isLoggingInChanged)

    /**
     * @brief Whether login has successfully completed.
     */
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)

    /**
     * @brief Whether the password (or the username) is invalid.
     */
    Q_PROPERTY(bool isInvalidPassword READ isInvalidPassword NOTIFY isInvalidPasswordChanged)

public:
    explicit LoginHelper(QObject *parent = nullptr);

    Q_INVOKABLE void init();

    bool homeserverReachable() const;

    QString matrixId() const;
    void setMatrixId(const QString &matrixId);

    QString password() const;
    void setPassword(const QString &password);

    QString deviceName() const;
    void setDeviceName(const QString &deviceName);

    bool supportsPassword() const;
    bool supportsSso() const;

    bool testing() const;

    QUrl ssoUrl() const;

    bool isLoggingIn() const;

    bool isLoggedIn() const;

    bool isInvalidPassword() const;
    void setInvalidPassword(bool invalid);

    Q_INVOKABLE void login();
    Q_INVOKABLE void loginWithSso();

Q_SIGNALS:
    void homeserverReachableChanged();
    void testHomeserverFinished();
    void matrixIdChanged();
    void passwordChanged();
    void deviceNameChanged();
    void loginFlowsChanged();
    void ssoUrlChanged();
    void connected();
    void errorOccured(const QString &message);
    void testingChanged();
    void isLoggingInChanged();
    void isLoggedInChanged();
    void isInvalidPasswordChanged();

private:
    void setHomeserverReachable(bool reachable);

    bool m_homeserverReachable;
    QString m_matrixId;
    QString m_password;
    QString m_deviceName;
    bool m_supportsSso = false;
    bool m_supportsPassword = false;
    NeoChatConnection *m_connection = nullptr;
    QUrl m_ssoUrl;
    bool m_testing = false;
    bool m_isLoggingIn = false;
    bool m_isLoggedIn = false;
    bool m_invalidPassword = false;
};
