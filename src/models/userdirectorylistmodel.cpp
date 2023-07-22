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

    m_limited = false;
    attempted = false;
    users.clear();

    if (m_connection) {
        m_connection->disconnect(this);
    }

    endResetModel();

    m_connection = conn;

    if (job) {
        job->abandon();
        job = nullptr;
    }

    Q_EMIT connectionChanged();
    Q_EMIT limitedChanged();
}

QString UserDirectoryListModel::keyword() const
{
    return m_keyword;
}

void UserDirectoryListModel::setKeyword(const QString &value)
{
    if (m_keyword == value) {
        return;
    }

    m_keyword = value;

    m_limited = false;
    attempted = false;

    if (job) {
        job->abandon();
        job = nullptr;
    }

    Q_EMIT keywordChanged();
    Q_EMIT limitedChanged();
}

bool UserDirectoryListModel::limited() const
{
    return m_limited;
}

void UserDirectoryListModel::search(int count)
{
    if (count < 1) {
        return;
    }

    if (job) {
        qDebug() << "UserDirectoryListModel: Other jobs running, ignore";

        return;
    }

    if (attempted) {
        return;
    }

    job = m_connection->callApi<SearchUserDirectoryJob>(m_keyword, count);

    connect(job, &BaseJob::finished, this, [this] {
        attempted = true;

        if (job->status() == BaseJob::Success) {
            auto users = job->results();

            this->beginResetModel();

            this->users = users;
            this->m_limited = job->limited();

            this->endResetModel();
        }

        this->job = nullptr;

        Q_EMIT limitedChanged();
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
    if (role == NameRole) {
        auto displayName = user.displayName;
        if (!displayName.isEmpty()) {
            return displayName;
        }

        displayName = user.userId;
        if (!displayName.isEmpty()) {
            return displayName;
        }

        return "Unknown User";
    }
    if (role == AvatarRole) {
        auto avatarUrl = user.avatarUrl;

        if (avatarUrl.isEmpty()) {
            return QString();
        }
        return avatarUrl.url().remove(0, 6);
    }
    if (role == UserIDRole) {
        return user.userId;
    }
    if (role == DirectChatsRole) {
        if (!m_connection) {
            return QStringList();
        };

        auto userObj = m_connection->user(user.userId);
        auto directChats = m_connection->directChats();

        if (userObj && directChats.contains(userObj)) {
            auto directChatsForUser = directChats.values(userObj);
            if (!directChatsForUser.isEmpty()) {
                return QVariant::fromValue(directChatsForUser);
            }
        }

        return QStringList();
    }

    return {};
}

QHash<int, QByteArray> UserDirectoryListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[NameRole] = "name";
    roles[AvatarRole] = "avatar";
    roles[UserIDRole] = "userID";
    roles[DirectChatsRole] = "directChats";

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
