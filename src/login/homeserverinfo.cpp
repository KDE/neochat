// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "homeserverinfo.h"
#include "neochatconnection.h"
#include <Quotient/connection.h>

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
}

bool HomeserverInfo::canSso() const
{
    return m_testConnection && m_testConnection->getLoginFlow(LoginFlowTypes::SSO).has_value();
}

bool HomeserverInfo::canPassword() const
{
    return m_testConnection && m_testConnection->getLoginFlow(LoginFlowTypes::Password).has_value();
}

QString HomeserverInfo::ssoUrl()
{
    if (!m_testConnection) {
        return {};
    }
    if (!m_ssoSession) {
        m_ssoSession = m_testConnection->prepareForSso(u"NeoChat"_s);
    }
    return m_ssoSession->ssoUrl().toString();
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
    auto username = matrixId.mid(1, matrixId.indexOf(QLatin1Char(':')) - 1);
    auto connection = std::make_unique<NeoChatConnection>();
    connection->loginWithPassword(username, password, {}, {});
}
