// SPDX-FileCopyrightText: 2019-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "publicroomlistmodel.h"

PublicRoomListModel::PublicRoomListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void PublicRoomListModel::setConnection(Connection *conn)
{
    if (m_connection == conn) {
        return;
    }

    beginResetModel();

    nextBatch = "";
    attempted = false;
    rooms.clear();
    m_server.clear();

    if (m_connection) {
        m_connection->disconnect(this);
    }

    endResetModel();

    m_connection = conn;

    if (job) {
        job->abandon();
        job = nullptr;
    }

    if (m_connection) {
        next();
    }

    Q_EMIT connectionChanged();
    Q_EMIT serverChanged();
    Q_EMIT hasMoreChanged();
}

void PublicRoomListModel::setServer(const QString &value)
{
    if (m_server == value) {
        return;
    }

    m_server = value;

    beginResetModel();

    nextBatch = "";
    attempted = false;
    rooms.clear();

    endResetModel();

    if (job) {
        job->abandon();
        job = nullptr;
    }

    if (m_connection) {
        next();
    }

    Q_EMIT serverChanged();
    Q_EMIT hasMoreChanged();
}

void PublicRoomListModel::setKeyword(const QString &value)
{
    if (m_keyword == value) {
        return;
    }

    m_keyword = value;

    beginResetModel();

    nextBatch = "";
    attempted = false;
    rooms.clear();

    endResetModel();

    if (job) {
        job->abandon();
        job = nullptr;
    }

    if (m_connection) {
        next();
    }

    Q_EMIT keywordChanged();
    Q_EMIT hasMoreChanged();
}

void PublicRoomListModel::next(int count)
{
    if (count < 1) {
        return;
    }

    if (job) {
        qDebug() << "PublicRoomListModel: Other jobs running, ignore";

        return;
    }

    job = m_connection->callApi<QueryPublicRoomsJob>(m_server, count, nextBatch, QueryPublicRoomsJob::Filter{m_keyword});

    connect(job, &BaseJob::finished, this, [this] {
        attempted = true;

        if (job->status() == BaseJob::Success) {
            nextBatch = job->nextBatch();

            this->beginInsertRows({}, rooms.count(), rooms.count() + job->chunk().count() - 1);
            rooms.append(job->chunk());
            this->endInsertRows();

            if (job->nextBatch().isEmpty()) {
                Q_EMIT hasMoreChanged();
            }
        }

        this->job = nullptr;
    });
}

QVariant PublicRoomListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= rooms.count()) {
        qDebug() << "PublicRoomListModel, something's wrong: index.row() >= "
                    "rooms.count()";
        return {};
    }
    auto room = rooms.at(index.row());
    if (role == NameRole) {
        auto displayName = room.name;
        if (!displayName.isEmpty()) {
            return displayName;
        }

        displayName = room.canonicalAlias;
        if (!displayName.isEmpty()) {
            return displayName;
        }

        if (!room.aliases.isEmpty()) {
            displayName = room.aliases.front();
        }

        if (!displayName.isEmpty()) {
            return displayName;
        }

        return room.roomId;
    }
    if (role == AvatarRole) {
        auto avatarUrl = room.avatarUrl;

        if (avatarUrl.isEmpty()) {
            return "";
        }
#ifdef QUOTIENT_07
        return avatarUrl.url().remove(0, 6);
#else
        return avatarUrl.remove(0, 6);
#endif
    }
    if (role == TopicRole) {
        return room.topic;
    }
    if (role == RoomIDRole) {
        return room.roomId;
    }
    if (role == AliasRole) {
        if (!room.canonicalAlias.isEmpty()) {
            return room.canonicalAlias;
        }
        return {};
    }
    if (role == MemberCountRole) {
        return room.numJoinedMembers;
    }
    if (role == AllowGuestsRole) {
        return room.guestCanJoin;
    }
    if (role == WorldReadableRole) {
        return room.worldReadable;
    }
    if (role == IsJoinedRole) {
        if (!m_connection) {
            return {};
        }

        return m_connection->room(room.roomId, JoinState::Join) != nullptr;
    }

    return {};
}

QHash<int, QByteArray> PublicRoomListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[NameRole] = "name";
    roles[AvatarRole] = "avatar";
    roles[TopicRole] = "topic";
    roles[RoomIDRole] = "roomID";
    roles[MemberCountRole] = "memberCount";
    roles[AllowGuestsRole] = "allowGuests";
    roles[WorldReadableRole] = "worldReadable";
    roles[IsJoinedRole] = "isJoined";
    roles[AliasRole] = "alias";

    return roles;
}

int PublicRoomListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return rooms.count();
}

bool PublicRoomListModel::hasMore() const
{
    return !(attempted && nextBatch.isEmpty());
}
