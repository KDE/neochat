// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "userlistmodel.h"

#include <QGuiApplication>

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
        m_currentRoom->disconnect(this);
        m_currentRoom->connection()->disconnect(this);
    }
    m_currentRoom = room;

    if (m_currentRoom) {
        connect(m_currentRoom, &Room::memberJoined, this, &UserListModel::memberJoined);
        connect(m_currentRoom, &Room::memberLeft, this, &UserListModel::memberLeft);
        connect(m_currentRoom, &Room::memberNameUpdated, this, [this](RoomMember member) {
            refreshMember(member, {DisplayNameRole});
        });
        connect(m_currentRoom, &Room::memberAvatarUpdated, this, [this](RoomMember member) {
            refreshMember(member, {AvatarRole});
        });
        connect(m_currentRoom, &Room::changed, this, &UserListModel::refreshAllMembers);
        connect(m_currentRoom->connection(), &Connection::loggedOut, this, [this]() {
            setRoom(nullptr);
        });
    }

    refreshAllMembers();
    Q_EMIT roomChanged();
}

NeoChatRoom *UserListModel::room() const
{
    return m_currentRoom;
}

QVariant UserListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_members.count()) {
        qDebug() << "UserListModel, something's wrong: index.row() >= "
                    "users.count()";
        return {};
    }
    auto member = m_members.at(index.row());
    if (role == DisplayNameRole) {
        return member.disambiguatedName();
    }
    if (role == UserIdRole) {
        return member.id();
    }
    if (role == AvatarRole) {
        return member.avatarUrl();
    }
    if (role == ObjectRole) {
        return QVariant::fromValue(member);
    }
    if (role == PowerLevelRole) {
        auto plEvent = m_currentRoom->currentState().get<RoomPowerLevelsEvent>();
        if (!plEvent) {
            return 0;
        }
        return plEvent->powerLevelForUser(member.id());
    }
    if (role == PowerLevelStringRole) {
        auto pl = m_currentRoom->currentState().get<RoomPowerLevelsEvent>();
        // User might not in the room yet, in this case pl can be nullptr.
        // e.g. When invited but user not accepted or denied the invitation.
        if (!pl) {
            return QStringLiteral("Not Available");
        }

        auto userPl = pl->powerLevelForUser(member.id());

        return i18nc("%1 is the name of the power level, e.g. admin and %2 is the value that represents.",
                     "%1 (%2)",
                     PowerLevel::nameForLevel(PowerLevel::levelForValue(userPl)),
                     userPl);
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
        refreshAllMembers();
    }
    return QObject::event(event);
}

void UserListModel::memberJoined(const Quotient::RoomMember &member)
{
    auto pos = findUserPos(member);
    beginInsertRows(QModelIndex(), pos, pos);
    m_members.insert(pos, member);
    endInsertRows();
}

void UserListModel::memberLeft(const Quotient::RoomMember &member)
{
    auto pos = findUserPos(member);
    if (pos != m_members.size()) {
        beginRemoveRows(QModelIndex(), pos, pos);
        m_members.removeAt(pos);
        endRemoveRows();
    } else {
        qWarning() << "Trying to remove a room member not in the user list";
    }
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
    m_members.clear();

    if (m_currentRoom != nullptr) {
        m_members = m_currentRoom->members();
        std::sort(m_members.begin(), m_members.end(), m_currentRoom->memberSorter());
    }
    endResetModel();
    Q_EMIT usersRefreshed();
}

int UserListModel::findUserPos(const RoomMember &member) const
{
    return findUserPos(member.displayName());
}

int UserListModel::findUserPos(const QString &username) const
{
    if (!m_currentRoom) {
        return 0;
    }
    return m_currentRoom->memberSorter().lowerBoundIndex(m_members, username);
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

    return roles;
}

#include "moc_userlistmodel.cpp"
