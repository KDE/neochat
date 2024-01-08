// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "serverlistmodel.h"

#include <Quotient/connection.h>

#include <QDebug>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

#include "neochatconnection.h"

ServerListModel::ServerListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant ServerListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() >= m_servers.count()) {
        qDebug() << "ServerListModel, something's wrong: index.row() >= m_notificationRules.count()";
        return {};
    }

    if (role == UrlRole) {
        return m_servers.at(index.row()).url;
    }

    if (role == IsHomeServerRole) {
        return m_servers.at(index.row()).isHomeServer;
    }

    if (role == IsAddServerDelegateRole) {
        return m_servers.at(index.row()).isAddServerDelegate;
    }

    if (role == IsDeletableRole) {
        return m_servers.at(index.row()).isDeletable;
    }

    return {};
}

int ServerListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_servers.count();
}

void ServerListModel::checkServer(const QString &url)
{
    const auto stateConfig = KSharedConfig::openStateConfig();
    const KConfigGroup serverGroup = stateConfig->group(QStringLiteral("Servers"));

    if (!serverGroup.hasKey(url)) {
        if (Quotient::isJobPending(m_checkServerJob)) {
            m_checkServerJob->abandon();
        }

        m_checkServerJob = m_connection->callApi<Quotient::QueryPublicRoomsJob>(url, 1);
        connect(m_checkServerJob, &Quotient::BaseJob::success, this, [this, url] {
            Q_EMIT serverCheckComplete(url, true);
        });
    }
}

void ServerListModel::addServer(const QString &url)
{
    const auto stateConfig = KSharedConfig::openStateConfig();
    KConfigGroup serverGroup = stateConfig->group(QStringLiteral("Servers"));

    if (!serverGroup.hasKey(url)) {
        Server newServer = Server{
            url,
            false,
            false,
            true,
        };

        beginInsertRows(QModelIndex(), m_servers.count() - 1, m_servers.count() - 1);
        m_servers.insert(rowCount() - 1, newServer);
        endInsertRows();
    }

    serverGroup.writeEntry(url, url);
    stateConfig->sync();
}

void ServerListModel::removeServerAtIndex(int row)
{
    const auto stateConfig = KSharedConfig::openStateConfig();
    KConfigGroup serverGroup = stateConfig->group(QStringLiteral("Servers"));

    serverGroup.deleteEntry(data(index(row), UrlRole).toString());

    beginRemoveRows(QModelIndex(), row, row);
    m_servers.removeAt(row);
    endRemoveRows();

    stateConfig->sync();
}

QHash<int, QByteArray> ServerListModel::roleNames() const
{
    return {
        {UrlRole, QByteArrayLiteral("url")},
        {IsHomeServerRole, QByteArrayLiteral("isHomeServer")},
        {IsAddServerDelegateRole, QByteArrayLiteral("isAddServerDelegate")},
        {IsDeletableRole, QByteArrayLiteral("isDeletable")},
    };
}

NeoChatConnection *ServerListModel::connection() const
{
    return m_connection;
}

void ServerListModel::setConnection(NeoChatConnection *connection)
{
    if (m_connection == connection) {
        return;
    }
    m_connection = connection;
    Q_EMIT connectionChanged();

    initialize();
}

void ServerListModel::initialize()
{
    if (m_connection == nullptr) {
        return;
    }

    beginResetModel();
    const auto stateConfig = KSharedConfig::openStateConfig();
    const KConfigGroup serverGroup = stateConfig->group(QStringLiteral("Servers"));

    QString domain = m_connection->domain();

    // Add the user's homeserver
    m_servers.append(Server{
        domain,
        true,
        false,
        false,
    });
    // Add matrix.org
    m_servers.append(Server{
        QStringLiteral("matrix.org"),
        false,
        false,
        false,
    });
    // Add each of the saved custom servers
    for (const auto &i : serverGroup.keyList()) {
        m_servers.append(Server{
            serverGroup.readEntry(i, QString()),
            false,
            false,
            true,
        });
    }
    // Add add server delegate entry
    m_servers.append(Server{
        QString(),
        false,
        true,
        false,
    });
    beginResetModel();
}

#include "moc_serverlistmodel.cpp"
