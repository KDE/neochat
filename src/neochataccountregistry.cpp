// SPDX-FileCopyrightText: Kitsune Ral <Kitsune-Ral@users.sf.net>
// SPDX-FileCopyrightText: Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "neochataccountregistry.h"

#include <connection.h>

using namespace Quotient;

void AccountRegistry::add(Connection *c)
{
    if (m_accounts.contains(c))
        return;
    beginInsertRows(QModelIndex(), m_accounts.size(), m_accounts.size());
    m_accounts += c;
    endInsertRows();
    emit accountCountChanged();
}

void AccountRegistry::drop(Connection *c)
{
    beginRemoveRows(QModelIndex(), m_accounts.indexOf(c), m_accounts.indexOf(c));
    m_accounts.removeOne(c);
    endRemoveRows();
    Q_ASSERT(!m_accounts.contains(c));
    emit accountCountChanged();
}

bool AccountRegistry::isLoggedIn(const QString &userId) const
{
    return std::any_of(m_accounts.cbegin(), m_accounts.cend(), [&userId](Connection *a) {
        return a->userId() == userId;
    });
}

bool AccountRegistry::contains(Connection *c) const
{
    return m_accounts.contains(c);
}

AccountRegistry::AccountRegistry() = default;

QVariant AccountRegistry::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() >= m_accounts.count()) {
        return {};
    }

    const auto account = m_accounts[index.row()];

    switch (role) {
    case ConnectionRole:
        return QVariant::fromValue(account);
    case UserIdRole:
        return QVariant::fromValue(account->userId());
    default:
        return {};
    }

    return {};
}

int AccountRegistry::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_accounts.count();
}

QHash<int, QByteArray> AccountRegistry::roleNames() const
{
    return {{ConnectionRole, "connection"}, {UserIdRole, "userId"}};
}

bool AccountRegistry::isEmpty() const
{
    return m_accounts.isEmpty();
}

int AccountRegistry::count() const
{
    return m_accounts.count();
}

const QVector<Connection *> AccountRegistry::accounts() const
{
    return m_accounts;
}

Connection *AccountRegistry::get(const QString &userId)
{
    for (const auto &connection : m_accounts) {
        if (connection->userId() == userId) {
            return connection;
        }
    }
    return nullptr;
}
