// SPDX-FileCopyrightText: 2019-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "publicroomlistmodel.h"

#include "neochatconnection.h"
#include "publicroomlist_logging.h"

using namespace Quotient;

class NeoChatQueryPublicRoomsJob : public QueryPublicRoomsJob
{
public:
    explicit NeoChatQueryPublicRoomsJob(const QString &server = {},
                                        std::optional<int> limit = std::nullopt,
                                        const QString &since = {},
                                        const std::optional<Filter> &filter = std::nullopt,
                                        std::optional<bool> includeAllNetworks = std::nullopt,
                                        const QString &thirdPartyInstanceId = {})
        : QueryPublicRoomsJob(server, limit, since, filter, includeAllNetworks, thirdPartyInstanceId)
    {
        // TODO Remove once we can use libQuotient's job directly
        // This is to make libQuotient happy about results not having the "chunk" field
        setExpectedKeys({});
    }
};

PublicRoomListModel::PublicRoomListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

NeoChatConnection *PublicRoomListModel::connection() const
{
    return m_connection;
}

void PublicRoomListModel::setConnection(NeoChatConnection *connection)
{
    if (m_connection == connection) {
        return;
    }

    beginResetModel();

    nextBatch = QString();
    attempted = false;
    rooms.clear();
    m_server.clear();

    if (m_connection) {
        m_connection->disconnect(this);
    }

    endResetModel();

    m_connection = connection;

    if (job) {
        job->abandon();
        job = nullptr;
        Q_EMIT searchingChanged();
    }

    if (m_connection) {
        next();
    }

    Q_EMIT connectionChanged();
    Q_EMIT serverChanged();
}

QString PublicRoomListModel::server() const
{
    return m_server;
}

void PublicRoomListModel::setServer(const QString &value)
{
    if (m_server == value) {
        return;
    }

    m_server = value;

    beginResetModel();

    nextBatch = QString();
    attempted = false;
    rooms.clear();

    endResetModel();

    if (job) {
        job->abandon();
        job = nullptr;
        Q_EMIT searchingChanged();
    }

    if (m_connection) {
        next();
    }

    Q_EMIT serverChanged();
}

QString PublicRoomListModel::searchText() const
{
    return m_searchText;
}

void PublicRoomListModel::setSearchText(const QString &value)
{
    if (m_searchText == value) {
        return;
    }

    m_searchText = value;
    Q_EMIT searchTextChanged();

    nextBatch = QString();
    attempted = false;

    if (job) {
        job->abandon();
        job = nullptr;
        Q_EMIT searchingChanged();
    }
}

bool PublicRoomListModel::showOnlySpaces() const
{
    return m_showOnlySpaces;
}

void PublicRoomListModel::setShowOnlySpaces(bool showOnlySpaces)
{
    if (showOnlySpaces == m_showOnlySpaces) {
        return;
    }
    m_showOnlySpaces = showOnlySpaces;
    Q_EMIT showOnlySpacesChanged();

    nextBatch = QString();
    attempted = false;

    if (job) {
        job->abandon();
        job = nullptr;
        Q_EMIT searchingChanged();
    }
}

void PublicRoomListModel::search(int limit)
{
    if (limit < 1 || attempted) {
        return;
    }

    if (job) {
        qCDebug(PublicRoomList) << "Other job running, ignore";
        return;
    }

    next(limit);
}

void PublicRoomListModel::next(int limit)
{
    if (m_connection == nullptr || limit < 1) {
        return;
    }
    m_redirectedText.clear();
    Q_EMIT redirectedChanged();

    if (job) {
        qCDebug(PublicRoomList) << "Other job running, ignore";
        return;
    }

    QStringList roomTypes;
    if (m_showOnlySpaces) {
        roomTypes += QLatin1String("m.space");
    }
    job = m_connection->callApi<NeoChatQueryPublicRoomsJob>(m_server, limit, nextBatch, QueryPublicRoomsJob::Filter{m_searchText, roomTypes});
    Q_EMIT searchingChanged();

    connect(job, &BaseJob::finished, this, [this] {
        if (!attempted) {
            beginResetModel();
            rooms.clear();
            endResetModel();

            attempted = true;
        }

        if (job->status() == BaseJob::Success) {
            nextBatch = job->nextBatch();

            this->beginInsertRows({}, rooms.count(), rooms.count() + job->chunk().count() - 1);
            rooms.append(job->chunk());
            this->endInsertRows();
        } else if (job->error() == BaseJob::ContentAccessError) {
            m_redirectedText = job->jsonData()[u"error"_s].toString();
            Q_EMIT redirectedChanged();
        }

        this->job = nullptr;
        Q_EMIT searchingChanged();
    });
}

QVariant PublicRoomListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= rooms.count()) {
        qCDebug(PublicRoomList) << "something's wrong: index.row() >= rooms.count()";
        return {};
    }
    auto room = rooms.at(index.row());
    if (role == DisplayNameRole) {
        auto displayName = room.name;
        if (!displayName.isEmpty()) {
            return displayName;
        }

        displayName = room.canonicalAlias;
        if (!displayName.isEmpty()) {
            return displayName;
        }

        if (!displayName.isEmpty()) {
            return displayName;
        }

        return room.roomId;
    }
    if (role == AvatarUrlRole) {
        auto avatarUrl = room.avatarUrl;
        if (avatarUrl.isEmpty() || !m_connection) {
            return QUrl();
        }
        return m_connection->makeMediaUrl(avatarUrl);
    }
    if (role == TopicRole) {
        return room.topic;
    }
    if (role == RoomIdRole) {
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
    if (role == IsSpaceRole) {
        return room.roomType == QLatin1String("m.space");
    }

    return {};
}

QHash<int, QByteArray> PublicRoomListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[DisplayNameRole] = "displayName";
    roles[AvatarUrlRole] = "avatarUrl";
    roles[TopicRole] = "topic";
    roles[RoomIdRole] = "roomId";
    roles[MemberCountRole] = "memberCount";
    roles[AllowGuestsRole] = "allowGuests";
    roles[WorldReadableRole] = "worldReadable";
    roles[IsJoinedRole] = "isJoined";
    roles[IsSpaceRole] = "isSpace";
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

bool PublicRoomListModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return !nextBatch.isEmpty();
}

void PublicRoomListModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    next();
}

bool PublicRoomListModel::searching() const
{
    return job != nullptr;
}

QString PublicRoomListModel::redirectedText() const
{
    return m_redirectedText;
}

#include "moc_publicroomlistmodel.cpp"
