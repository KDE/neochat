// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "homeserverinfo.h"
#include "neochatconnection.h"

#include <Quotient/connection.h>
#include <Quotient/settings.h>

#include <KLocalizedString>

using namespace Quotient;

HomeserverInfo::HomeserverInfo(QObject *parent)
    : QObject(parent)
{
    connect(this, &HomeserverInfo::homeserverChanged, this, &HomeserverInfo::test);
}

void HomeserverInfo::setHomeserver(const QString &homeserver)
{
    if (m_homeserver == homeserver) {
        return;
    }
    m_homeserver = homeserver;
    Q_EMIT homeserverChanged();
}

QString HomeserverInfo::homeserver() const
{
    return m_homeserver;
}

void HomeserverInfo::test()
{
    // TODO don't delete connection if we actually start doing something with it (i.e., if the user types something after already starting sso process)
    delete m_testConnection;
    delete m_ssoSession;
    setReachable(false);
    Q_EMIT flowsChanged();
    m_testConnection = new NeoChatConnection(this);
    m_testConnection->resolveServer("@user:%1"_L1.arg(m_homeserver));
    connect(m_testConnection.get(), &NeoChatConnection::loginFlowsChanged, this, [this] {
        setReachable(true);
        Q_EMIT flowsChanged();
    });

    connect(m_testConnection, &Connection::connected, this, [this] {
        setLoggingIn(false);
        AccountSettings account(m_testConnection->userId());
        account.setKeepLoggedIn(true);
        account.setHomeserver(m_testConnection->homeserver());
        account.setDeviceId(m_testConnection->deviceId());
        account.setDeviceName({});
        account.sync();
        // TODO m_accountManager->addConnection(m_connection);
        // TODO m_accountManager->setActiveConnection(m_connection);
        disconnect(m_testConnection, nullptr, this, nullptr);
        // TODO use syncDone directly in the ui for that
    });
    connect(m_testConnection, &NeoChatConnection::networkError, this, [this](const auto &error, const auto &, int, int) {
        setLoggingIn(false);
        Q_EMIT m_testConnection->errorOccured(i18n("Network Error: %1", std::move(error)));
    });
    connect(m_testConnection, &NeoChatConnection::loginError, this, [this](const auto &error, const auto &) {
        setLoggingIn(false);
        if (error == u"Invalid username or password"_s) {
            // setInvalidPassword(true); // TODO do in NeoChatConnection
        } else {
            // Q_EMIT loginErrorOccured(i18n("Login Failed: %1", error));
        }
    });

    connect(m_testConnection, &NeoChatConnection::resolveError, this, [this](const auto error) {
        setLoggingIn(false);
        Q_EMIT m_testConnection->errorOccured(i18nc("@info", "Network Error: %1", std::move(error)));
    });
}

bool HomeserverInfo::canSso() const
{
    return m_testConnection && m_testConnection->getLoginFlow(LoginFlowTypes::SSO).has_value();
}

bool HomeserverInfo::canPassword() const
{
    return m_testConnection && m_testConnection->getLoginFlow(LoginFlowTypes::Password).has_value();
}

QUrl HomeserverInfo::ssoUrl()
{
    if (!m_testConnection) {
        return {};
    }
    if (!m_ssoSession) {
        m_ssoSession = m_testConnection->prepareForSso(u"NeoChat"_s);
    }
    return m_ssoSession->ssoUrl();
}

bool HomeserverInfo::reachable() const
{
    return m_reachable;
}

void HomeserverInfo::setReachable(bool reachable)
{
    if (m_reachable == reachable) {
        return;
    }
    m_reachable = reachable;
    Q_EMIT reachableChanged();
}

void HomeserverInfo::loginWithPassword(const QString &matrixId, const QString &password)
{
    setLoggingIn(true);
    // TODO ensure this only runs once
    m_testConnection->loginWithPassword(matrixId.mid(1, matrixId.indexOf(QLatin1Char(':')) - 1), password, {}, {});
}

void HomeserverInfo::setLoggingIn(bool loggingIn)
{
    if (m_loggingIn == loggingIn) {
        return;
    }

    m_loggingIn = loggingIn;
    Q_EMIT loggingInChanged();
}

bool HomeserverInfo::loggingIn() const
{
    return m_loggingIn;
}
