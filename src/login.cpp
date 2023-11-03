// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "login.h"

#include <Quotient/accountregistry.h>
#include <Quotient/connection.h>
#include <Quotient/qt_connection_util.h>

#include "controller.h"

#include <KLocalizedString>

using namespace Quotient;

LoginHelper::LoginHelper(QObject *parent)
    : QObject(parent)
{
    init();
}

void LoginHelper::init()
{
    m_homeserverReachable = false;
    m_connection = new NeoChatConnection();
    m_matrixId = QString();
    m_password = QString();
    m_deviceName = QStringLiteral("NeoChat %1 %2 %3 %4")
                       .arg(QSysInfo::machineHostName(), QSysInfo::productType(), QSysInfo::productVersion(), QSysInfo::currentCpuArchitecture());
    m_supportsSso = false;
    m_supportsPassword = false;
    m_ssoUrl = QUrl();

    connect(this, &LoginHelper::matrixIdChanged, this, [this]() {
        setHomeserverReachable(false);
        QRegularExpression validator(QStringLiteral("^\\@?[a-zA-Z0-9\\._=\\-/]+\\:[a-zA-Z0-9\\-]+(\\.[a-zA-Z0-9\\-]+)*(\\:[0-9]+)?$"));
        if (!validator.match(m_matrixId).hasMatch()) {
            return;
        }

        if (m_matrixId == QLatin1Char('@')) {
            return;
        }

        m_isLoggedIn = Controller::instance().accounts().isLoggedIn(m_matrixId);
        Q_EMIT isLoggedInChanged();
        if (m_isLoggedIn) {
            return;
        }

        m_testing = true;
        Q_EMIT testingChanged();
        if (!m_connection) {
            m_connection = new NeoChatConnection();
        }
        m_connection->resolveServer(m_matrixId);
        connectSingleShot(m_connection, &Connection::loginFlowsChanged, this, [this]() {
            setHomeserverReachable(true);
            m_testing = false;
            Q_EMIT testingChanged();
            m_supportsSso = m_connection->supportsSso();
            m_supportsPassword = m_connection->supportsPasswordAuth();
            Q_EMIT loginFlowsChanged();
        });
    });
    connect(m_connection, &Connection::connected, this, [this] {
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
    connect(m_connection, &Connection::networkError, this, [this](QString error, const QString &, int, int) {
        Q_EMIT Controller::instance().globalErrorOccured(i18n("Network Error"), std::move(error));
        m_isLoggingIn = false;
        Q_EMIT isLoggingInChanged();
    });
    connect(m_connection, &Connection::loginError, this, [this](QString error, const QString &) {
        if (error == QStringLiteral("Invalid username or password")) {
            setInvalidPassword(true);
        } else {
            Q_EMIT errorOccured(i18n("Login Failed: %1", error));
        }
        m_isLoggingIn = false;
        Q_EMIT isLoggingInChanged();
    });

    connect(m_connection, &Connection::resolveError, this, [](QString error) {
        Q_EMIT Controller::instance().globalErrorOccured(i18n("Network Error"), std::move(error));
    });

    connectSingleShot(m_connection, &Connection::syncDone, this, [this]() {
        Q_EMIT loaded();
        Q_EMIT Controller::instance().initiated();
    });
}

void LoginHelper::setHomeserverReachable(bool reachable)
{
    m_homeserverReachable = reachable;
    Q_EMIT homeserverReachableChanged();
}

bool LoginHelper::homeserverReachable() const
{
    return m_homeserverReachable;
}

QString LoginHelper::matrixId() const
{
    return m_matrixId;
}

void LoginHelper::setMatrixId(const QString &matrixId)
{
    m_matrixId = matrixId;
    if (!m_matrixId.startsWith(QLatin1Char('@'))) {
        m_matrixId.prepend(QLatin1Char('@'));
    }
    Q_EMIT matrixIdChanged();
}

QString LoginHelper::password() const
{
    return m_password;
}

void LoginHelper::setPassword(const QString &password)
{
    setInvalidPassword(false);
    m_password = password;
    Q_EMIT passwordChanged();
}

QString LoginHelper::deviceName() const
{
    return m_deviceName;
}

void LoginHelper::setDeviceName(const QString &deviceName)
{
    m_deviceName = deviceName;
    Q_EMIT deviceNameChanged();
}

void LoginHelper::login()
{
    m_isLoggingIn = true;
    Q_EMIT isLoggingInChanged();

    // Some servers do not have a .well_known file. So we login via the username part from the mxid,
    // rather than with the full mxid, as that would lead to an invalid user.
    auto username = m_matrixId.mid(1, m_matrixId.indexOf(QLatin1Char(':')) - 1);
    m_connection->loginWithPassword(username, m_password, m_deviceName, QString());
}

bool LoginHelper::supportsPassword() const
{
    return m_supportsPassword;
}

bool LoginHelper::supportsSso() const
{
    return m_supportsSso;
}

QUrl LoginHelper::ssoUrl() const
{
    return m_ssoUrl;
}

void LoginHelper::loginWithSso()
{
    m_connection->resolveServer(m_matrixId);
    connectSingleShot(m_connection, &Connection::loginFlowsChanged, this, [this]() {
        SsoSession *session = m_connection->prepareForSso(m_deviceName);
        m_ssoUrl = session->ssoUrl();
        Q_EMIT ssoUrlChanged();
    });
}

bool LoginHelper::testing() const
{
    return m_testing;
}

bool LoginHelper::isLoggingIn() const
{
    return m_isLoggingIn;
}

bool LoginHelper::isLoggedIn() const
{
    return m_isLoggedIn;
}

void LoginHelper::setInvalidPassword(bool invalid)
{
    m_invalidPassword = invalid;
    Q_EMIT isInvalidPasswordChanged();
}

bool LoginHelper::isInvalidPassword() const
{
    return m_invalidPassword;
}

#include "moc_login.cpp"
