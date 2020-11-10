/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "accountlistmodel.h"

#include "room.h"

AccountListModel::AccountListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_connections.clear();
    m_connections += Controller::instance().connections();

    connect(&Controller::instance(), &Controller::connectionAdded, this, [=](Connection *conn) {
        if (!conn) {
            return;
        }
        beginInsertRows(QModelIndex(), m_connections.count(), m_connections.count());
        m_connections.append(conn);
        endInsertRows();
    });
    connect(&Controller::instance(), &Controller::connectionDropped, this, [=](Connection *conn) {
        qDebug() << "Dropping connection" << conn->userId();
        if (!conn) {
            qDebug() << "Trying to remove null connection";
            return;
        }
        conn->disconnect(this);
        const auto it = std::find(m_connections.begin(), m_connections.end(), conn);
        if (it == m_connections.end())
            return; // Already deleted, nothing to do
        const int row = it - m_connections.begin();
        beginRemoveRows(QModelIndex(), row, row);
        m_connections.erase(it);
        endRemoveRows();
    });
}

QVariant AccountListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() >= m_connections.count()) {
        qDebug() << "AccountListModel, something's wrong: index.row() >= "
                    "m_users.count()";
        return {};
    }

    auto m_connection = m_connections.at(index.row());

    if (role == UserRole) {
        return QVariant::fromValue(m_connection->user());
    }
    if (role == ConnectionRole) {
        return QVariant::fromValue(m_connection);
    }

    return {};
}

int AccountListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_connections.count();
}

QHash<int, QByteArray> AccountListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[UserRole] = "user";
    roles[ConnectionRole] = "connection";

    return roles;
}
