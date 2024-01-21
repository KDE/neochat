// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "userdirectorylistmodel.h"

#include <Quotient/connection.h>
#include <Quotient/room.h>

using namespace Quotient;

UserDirectoryListModel::UserDirectoryListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

Quotient::Connection *UserDirectoryListModel::connection() const
{
    return m_connection;
}

void UserDirectoryListModel::setConnection(Connection *conn)
{
    if (m_connection == conn) {
        return;
    }

    beginResetModel();

    attempted = false;
    users.clear();

    if (m_connection) {
        m_connection->disconnect(this);
    }

    endResetModel();

    m_connection = conn;
    Q_EMIT connectionChanged();

    if (m_job) {
        m_job->abandon();
        m_job = nullptr;
        Q_EMIT searchingChanged();
    }
}

QString UserDirectoryListModel::searchText() const
{
    return m_searchText;
}

void UserDirectoryListModel::setSearchText(const QString &value)
{
    if (m_searchText == value) {
        return;
    }

    m_searchText = value;
    Q_EMIT searchTextChanged();

    if (m_job) {
        m_job->abandon();
        m_job = nullptr;
        Q_EMIT searchingChanged();
    }

    attempted = false;
}

bool UserDirectoryListModel::searching() const
{
    return m_job != nullptr;
}

void UserDirectoryListModel::search(int limit)
{
    if (limit < 1) {
        return;
    }

    if (m_job) {
        qDebug() << "UserDirectoryListModel: Other jobs running, ignore";

        return;
    }

    if (attempted) {
        return;
    }

    m_job = m_connection->callApi<SearchUserDirectoryJob>(m_searchText, limit);
    Q_EMIT searchingChanged();

    connect(m_job, &BaseJob::finished, this, [this] {
        attempted = true;

        if (m_job->status() == BaseJob::Success) {
            auto users = m_job->results();

            this->beginResetModel();
            this->users = users;
            this->endResetModel();
        }

        this->m_job = nullptr;
        Q_EMIT searchingChanged();
    });
}

QVariant UserDirectoryListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= users.count()) {
        qDebug() << "UserDirectoryListModel, something's wrong: index.row() >= "
                    "users.count()";
        return {};
    }
    auto user = users.at(index.row());
    if (role == DisplayNameRole) {
        auto displayName = user.displayName;
        if (!displayName.isEmpty()) {
            return displayName;
        }

        displayName = user.userId;
        if (!displayName.isEmpty()) {
            return displayName;
        }

        return QStringLiteral("Unknown User");
    }
    if (role == AvatarRole) {
        auto avatarUrl = user.avatarUrl;
        if (avatarUrl.isEmpty() || !m_connection) {
            return QUrl();
        }
        return m_connection->makeMediaUrl(avatarUrl);
    }
    if (role == UserIDRole) {
        return user.userId;
    }
    if (role == DirectChatExistsRole) {
        if (!m_connection) {
            return false;
        };

        auto userObj = m_connection->user(user.userId);
        auto directChats = m_connection->directChats();

        if (userObj && directChats.contains(userObj)) {
            auto directChatsForUser = directChats.values(userObj);
            if (!directChatsForUser.isEmpty()) {
                return true;
            }
        }

        return false;
    }

    return {};
}

QHash<int, QByteArray> UserDirectoryListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[DisplayNameRole] = "displayName";
    roles[AvatarRole] = "avatarUrl";
    roles[UserIDRole] = "userId";
    roles[DirectChatExistsRole] = "directChatExists";

    return roles;
}

int UserDirectoryListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return users.count();
}

#include "moc_userdirectorylistmodel.cpp"
