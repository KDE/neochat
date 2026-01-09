// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "readmarkermodel.h"

#include <KLocalizedString>

#include <Quotient/roommember.h>

using namespace Qt::StringLiterals;

ReadMarkerModel::ReadMarkerModel(const QString &eventId, NeoChatRoom *room)
    : QAbstractListModel(nullptr)
    , m_room(room)
    , m_eventId(eventId)
{
    Q_ASSERT(!m_eventId.isEmpty());
    Q_ASSERT(m_room != nullptr);

    connect(m_room, &NeoChatRoom::changed, this, [this](Quotient::Room::Changes changes) {
        if (m_room != nullptr && changes.testFlag(Quotient::Room::Change::Other)) {
            auto memberIds = m_room->userIdsAtEvent(m_eventId).values();
            if (memberIds == m_markerIds) {
                return;
            }

            beginResetModel();
            m_markerIds.clear();
            endResetModel();

            beginResetModel();
            memberIds.removeAll(m_room->localMember().id());
            m_markerIds = memberIds;
            endResetModel();

            Q_EMIT reactionUpdated();
        }
    });
    connect(m_room, &NeoChatRoom::memberNameUpdated, this, [this](Quotient::RoomMember member) {
        if (m_markerIds.contains(member.id())) {
            const auto memberIndex = index(m_markerIds.indexOf(member.id()));
            Q_EMIT dataChanged(memberIndex, memberIndex);
        }
    });
    connect(m_room, &NeoChatRoom::memberAvatarUpdated, this, [this](Quotient::RoomMember member) {
        if (m_markerIds.contains(member.id())) {
            const auto memberIndex = index(m_markerIds.indexOf(member.id()));
            Q_EMIT dataChanged(memberIndex, memberIndex);
        }
    });

    beginResetModel();
    auto userIds = m_room->userIdsAtEvent(m_eventId);
    userIds.remove(m_room->localMember().id());
    m_markerIds = userIds.values();
    endResetModel();

    Q_EMIT reactionUpdated();
}

QVariant ReadMarkerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() >= rowCount()) {
        qDebug() << "ReadMarkerModel, something's wrong: index.row() >= rowCount()";
        return {};
    }

    const auto member = m_room->member(m_markerIds.value(index.row()));

    if (role == DisplayNameRole) {
        return member.htmlSafeDisplayName();
    }

    if (role == AvatarUrlRole) {
        return member.avatarUrl();
    }

    if (role == ColorRole) {
        return member.color();
    }

    if (role == UserIdRole) {
        return member.id();
    }

    return {};
}

int ReadMarkerModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_markerIds.size();
}

QHash<int, QByteArray> ReadMarkerModel::roleNames() const
{
    return {
        {DisplayNameRole, "displayName"},
        {AvatarUrlRole, "avatarUrl"},
        {ColorRole, "memberColor"},
        {UserIdRole, "userId"},
    };
}

QString ReadMarkerModel::readMarkersString()
{
    /**
     * The string ends up in the form
     * "x users: user1DisplayName, user2DisplayName, etc."
     */
    QString readMarkersString = i18np("1 user: ", "%1 users: ", m_markerIds.size());
    for (const auto &memberId : m_markerIds) {
        auto member = m_room->member(memberId);
        QString displayName = member.htmlSafeDisambiguatedName();
        if (displayName.isEmpty()) {
            displayName = i18nc("A member who is not in the room has been requested.", "unknown member");
        }
        readMarkersString += displayName + i18nc("list separator", ", ");
    }
    readMarkersString.chop(2);
    return readMarkersString;
}

#include "moc_readmarkermodel.cpp"
