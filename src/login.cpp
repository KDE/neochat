// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "login.h"
#include "connection.h"
#include "controller.h"

#include <QUrl>

#include <KLocalizedString>

Login::Login(QObject *parent)
    : QObject(parent)
{
    init();
}

void Login::init()
{
    m_homeserverReachable = false;
    m_connection = new Connection();
    m_matrixId = QString();
    m_password = QString();
    m_deviceName = QStringLiteral("NeoChat %1 %2 %3 %4")
                       .arg(QSysInfo::machineHostName(), QSysInfo::productType(), QSysInfo::productVersion(), QSysInfo::currentCpuArchitecture());
    m_supportsSso = false;
    m_supportsPassword = false;
    m_ssoUrl = QUrl();

    connect(this, &Login::matrixIdChanged, this, [=]() {
        setHomeserverReachable(false);

        if (m_matrixId == "@") {
            return;
        }

        m_testing = true;
        Q_EMIT testingChanged();
        if (!m_connection) {
            m_connection = new Connection();
        }
        m_connection->resolveServer(m_matrixId);
        connect(m_connection, &Connection::loginFlowsChanged, this, [=]() {
            setHomeserverReachable(true);
            m_testing = false;
            Q_EMIT testingChanged();
            m_supportsSso = m_connection->supportsSso();
            m_supportsPassword = m_connection->supportsPasswordAuth();
            Q_EMIT loginFlowsChanged();
        });
    });
    connect(m_connection, &Connection::connected, this, [=] {
        Q_EMIT connected();
        m_isLoggingIn = false;
        Q_EMIT isLoggingInChanged();
        Q_ASSERT(m_connection);
        AccountSettings account(m_connection->userId());
        account.setKeepLoggedIn(true);
        account.setHomeserver(m_connection->homeserver());
        account.setDeviceId(m_connection->deviceId());
        account.setDeviceName(m_deviceName);
        if (!Controller::instance().saveAccessTokenToKeyChain(account, m_connection->accessToken())) {
            qWarning() << "Couldn't save access token";
        }
        account.sync();
        Controller::instance().addConnection(m_connection);
        Controller::instance().setActiveConnection(m_connection);
        m_connection = nullptr;
    });
    connect(m_connection, &Connection::networkError, this, [=](QString error, const QString &, int, int) {
        Q_EMIT Controller::instance().globalErrorOccured(i18n("Network Error"), std::move(error));
        m_isLoggingIn = false;
        Q_EMIT isLoggingInChanged();
    });
    connect(m_connection, &Connection::loginError, this, [=](QString error, const QString &) {
        Q_EMIT errorOccured(i18n("Login Failed: %1", error));
        m_isLoggingIn = false;
        Q_EMIT isLoggingInChanged();
    });

    connect(m_connection, &Connection::resolveError, this, [=](QString error) {
        Q_EMIT Controller::instance().globalErrorOccured(i18n("Network Error"), std::move(error));
    });

    connectSingleShot(m_connection, &Connection::syncDone, this, [=]() {
        Q_EMIT Controller::instance().initiated();
    });
}

void Login::setHomeserverReachable(bool reachable)
{
    m_homeserverReachable = reachable;
    Q_EMIT homeserverReachableChanged();
}

bool Login::homeserverReachable() const
{
    return m_homeserverReachable;
}

QString Login::matrixId() const
{
    return m_matrixId;
}

void Login::setMatrixId(const QString &matrixId)
{
    m_matrixId = matrixId;
    if (!m_matrixId.startsWith('@')) {
        m_matrixId.prepend('@');
    }
    Q_EMIT matrixIdChanged();
}

QString Login::password() const
{
    return m_password;
}

void Login::setPassword(const QString &password)
{
    m_password = password;
    Q_EMIT passwordChanged();
}

QString Login::deviceName() const
{
    return m_deviceName;
}

void Login::setDeviceName(const QString &deviceName)
{
    m_deviceName = deviceName;
    Q_EMIT deviceNameChanged();
}

void Login::login()
{
    m_isLoggingIn = true;
    Q_EMIT isLoggingInChanged();

    m_connection->loginWithPassword(m_matrixId, m_password, m_deviceName, QString());
}

bool Login::supportsPassword() const
{
    return m_supportsPassword;
}

bool Login::supportsSso() const
{
    return m_supportsSso;
}

QUrl Login::ssoUrl() const
{
    return m_ssoUrl;
}

void Login::loginWithSso()
{
    m_connection->resolveServer(m_matrixId);

    SsoSession *session = m_connection->prepareForSso(m_deviceName);
    m_ssoUrl = session->ssoUrl();
    Q_EMIT ssoUrlChanged();
}

bool Login::testing() const
{
    return m_testing;
}

bool Login::isLoggingIn() const
{
    return m_isLoggingIn;
}
