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
    delete m_testConnection;
    m_testConnection = new NeoChatConnection(this);
    m_testConnection->resolveServer("@user:%1"_L1.arg(m_homeserver));
    connect(m_testConnection.get(), &NeoChatConnection::loginFlowsChanged, this, &HomeserverInfo::flowsChanged);
}

bool HomeserverInfo::canSso() const
{
    return m_testConnection && m_testConnection->getLoginFlow(LoginFlowTypes::SSO).has_value();
}

bool HomeserverInfo::canPassword() const
{
    return m_testConnection && m_testConnection->getLoginFlow(LoginFlowTypes::Password).has_value();
}
