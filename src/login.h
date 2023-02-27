// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QUrl>

namespace Quotient
{
class Connection;
}
class Login : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool homeserverReachable READ homeserverReachable NOTIFY homeserverReachableChanged)
    Q_PROPERTY(bool testing READ testing NOTIFY testingChanged)
    Q_PROPERTY(QString matrixId READ matrixId WRITE setMatrixId NOTIFY matrixIdChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString deviceName READ deviceName WRITE setDeviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(bool supportsSso READ supportsSso NOTIFY loginFlowsChanged STORED false)
    Q_PROPERTY(bool supportsPassword READ supportsPassword NOTIFY loginFlowsChanged STORED false)
    Q_PROPERTY(QUrl ssoUrl READ ssoUrl NOTIFY ssoUrlChanged)
    Q_PROPERTY(bool isLoggingIn READ isLoggingIn NOTIFY isLoggingInChanged)
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)

public:
    explicit Login(QObject *parent = nullptr);

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
    void errorOccured(QString message);
    void testingChanged();
    void isLoggingInChanged();
    void isLoggedInChanged();

private:
    void setHomeserverReachable(bool reachable);

    bool m_homeserverReachable;
    QString m_matrixId;
    QString m_password;
    QString m_deviceName;
    bool m_supportsSso = false;
    bool m_supportsPassword = false;
    Quotient::Connection *m_connection = nullptr;
    QUrl m_ssoUrl;
    bool m_testing = false;
    bool m_isLoggingIn = false;
    bool m_isLoggedIn = false;
};
