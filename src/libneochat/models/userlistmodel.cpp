// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "userlistmodel.h"

#include <QGuiApplication>

#include <Quotient/avatar.h>
#include <Quotient/events/roompowerlevelsevent.h>

#include "enums/powerlevel.h"
#include "neochatroom.h"

using namespace Quotient;

UserListModel::UserListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_currentRoom(nullptr)
{
}

void UserListModel::setRoom(NeoChatRoom *room)
{
    if (m_currentRoom == room) {
        return;
    }

    if (m_currentRoom) {
        // HACK: Reset the model to a null room first to make sure QML dismantles
        // last room's objects before the room is actually changed
        beginResetModel();
        m_currentRoom->disconnect(this);
        m_currentRoom->connection()->disconnect(this);
        m_currentRoom = nullptr;
        m_members.clear();
        endResetModel();
    }

    m_currentRoom = room;

    if (m_currentRoom) {
        connect(m_currentRoom, &Room::memberJoined, this, &UserListModel::memberJoined);
        connect(m_currentRoom, &Room::memberNameUpdated, this, [this](RoomMember member) {
            refreshMember(member, {DisplayNameRole});
        });
        connect(m_currentRoom, &Room::memberAvatarUpdated, this, [this](RoomMember member) {
            refreshMember(member, {AvatarRole});
        });
        connect(m_currentRoom->connection(), &Connection::loggedOut, this, [this]() {
            setRoom(nullptr);
        });
    }

    m_active = false;
    Q_EMIT roomChanged();
}

NeoChatRoom *UserListModel::room() const
{
    return m_currentRoom;
}

QVariant UserListModel::data(const QModelIndex &index, int role) const
{
    if (!m_currentRoom) {
        return {};
    }
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_members.count()) {
        qDebug() << "UserListModel, something's wrong: index.row() >= "
                    "users.count()";
        return {};
    }
    const auto &memberId = m_members.at(index.row());
    if (role == DisplayNameRole) {
        return m_currentRoom->member(memberId).disambiguatedName();
    }
    if (role == UserIdRole) {
        return memberId;
    }
    if (role == AvatarRole) {
        return m_currentRoom->member(memberId).avatarUrl();
    }
    if (role == ObjectRole) {
        return QVariant::fromValue(memberId);
    }
    if (role == PowerLevelRole) {
        if (m_currentRoom->isCreator(memberId)) {
            return std::numeric_limits<int>::max();
        }
        auto plEvent = m_currentRoom->currentState().get<RoomPowerLevelsEvent>();
        if (!plEvent) {
            return 0;
        }
        return plEvent->powerLevelForUser(memberId);
    }
    if (role == PowerLevelStringRole) {
        if (m_currentRoom->roomCreatorHasUltimatePowerLevel() && m_currentRoom->isCreator(memberId)) {
            return i18nc("@info the person that created this room", "Creator");
        }

        auto pl = m_currentRoom->currentState().get<RoomPowerLevelsEvent>();
        // User might not in the room yet, in this case pl can be nullptr.
        // e.g. When invited but user not accepted or denied the invitation.
        if (!pl) {
            return u"Not Available"_s;
        }

        auto userPl = pl->powerLevelForUser(memberId);

        return i18nc("%1 is the name of the power level, e.g. admin and %2 is the value that represents.",
                     "%1 (%2)",
                     PowerLevel::nameForLevel(PowerLevel::levelForValue(userPl)),
                     userPl);
    }
    if (role == IsCreatorRole) {
        return m_currentRoom->isCreator(memberId);
    }
    if (role == MembershipRole) {
        return QVariant::fromValue(m_currentRoom->member(memberId).membershipState());
    }
    if (role == ColorRole) {
        return m_currentRoom->member(memberId).color();
    }

    return {};
}

int UserListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_members.count();
}

bool UserListModel::event(QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange) {
        // Quotient::RoomMember::color needs to be recalculated for the new palette
        Q_EMIT dataChanged(index(0, 0), index(m_members.size(), 0), {ColorRole});
    }
    return QObject::event(event);
}

void UserListModel::memberJoined(const Quotient::RoomMember &member)
{
    if (m_members.contains(member.id())) {
        return;
    }

    const int row = m_members.size();
    beginInsertRows(QModelIndex(), row, row);
    m_members.append(member.id());
    endInsertRows();
}

void UserListModel::refreshMember(const Quotient::RoomMember &member, const QList<int> &roles)
{
    auto pos = findUserPos(member);
    if (pos != m_members.size()) {
        Q_EMIT dataChanged(index(pos), index(pos), roles);
    } else {
        qWarning() << "Trying to access a room member not in the user list";
    }
}

void UserListModel::refreshAllMembers()
{
    beginResetModel();
    if (m_currentRoom != nullptr) {
        // Only sort members when this model needs to be refreshed.
        if (m_currentRoom->sortedMemberIds().isEmpty()) {
            m_currentRoom->sortAllMembers();
        }
        m_members = m_currentRoom->sortedMemberIds();
    } else {
        m_members.clear();
    }
    endResetModel();
    Q_EMIT usersRefreshed();
}

int UserListModel::findUserPos(const RoomMember &member) const
{
    return findUserPos(member.id());
}

int UserListModel::findUserPos(const QString &userId) const
{
    if (!m_currentRoom) {
        return 0;
    }
    const auto pos = std::find_if(m_members.cbegin(), m_members.cend(), [&userId](const QString &member) {
        return userId == member;
    });
    return pos - m_members.cbegin();
}

QHash<int, QByteArray> UserListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[DisplayNameRole] = "name";
    roles[UserIdRole] = "userId";
    roles[AvatarRole] = "avatar";
    roles[ObjectRole] = "user";
    roles[PowerLevelRole] = "powerLevel";
    roles[PowerLevelStringRole] = "powerLevelString";
    roles[IsCreatorRole] = "isCreator";
    roles[MembershipRole] = "membership";
    roles[ColorRole] = "color";

    return roles;
}

void UserListModel::activate()
{
    if (m_active) {
        return;
    }

    m_active = true;
    refreshAllMembers();
}

#include "moc_userlistmodel.cpp"
