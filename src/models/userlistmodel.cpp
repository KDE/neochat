// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "userlistmodel.h"

#include <QGuiApplication>

#include <Quotient/connection.h>
#include <Quotient/events/roompowerlevelsevent.h>

#include "neochatroom.h"

using namespace Quotient;

UserListModel::UserListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_currentRoom(nullptr)
{
    connect(static_cast<QGuiApplication *>(QGuiApplication::instance()), &QGuiApplication::paletteChanged, this, [this]() {
        refreshAllUsers();
    });
}

void UserListModel::setRoom(NeoChatRoom *room)
{
    if (m_currentRoom == room) {
        return;
    }

    if (m_currentRoom) {
        m_currentRoom->disconnect(this);
    }
    m_currentRoom = room;

    if (m_currentRoom) {
        connect(m_currentRoom, &Room::memberAdded, this, &UserListModel::memberAdded);
        connect(m_currentRoom, &Room::memberRemoved, this, &UserListModel::memberRemoved);
        connect(m_currentRoom, &Room::memberUpdated, this, &UserListModel::memberUpdated);
    }

    refreshAllUsers();
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

    if (index.row() >= rowCount()) {
        qDebug() << "UserListModel, something's wrong: index.row() >= "
                    "users.count()";
        return {};
    }
    auto user = m_members.at(index.row());
    if (role == DisplayNameRole) {
        return user->displayName();
    }
    if (role == UserIdRole) {
        return user->id();
    }
    if (role == AvatarRole) {
        return user->avatarUrl();
    }
    if (role == ObjectRole) {
        return QVariant::fromValue<RoomMember *>(user.get());
    }
    if (role == PowerLevelRole) {
        auto plEvent = m_currentRoom->currentState().get<RoomPowerLevelsEvent>();
        if (!plEvent) {
            return 0;
        }
        return plEvent->powerLevelForUser(user->id());
    }
    if (role == PowerLevelStringRole) {
        auto pl = m_currentRoom->currentState().get<RoomPowerLevelsEvent>();
        // User might not in the room yet, in this case pl can be nullptr.
        // e.g. When invited but user not accepted or denied the invitation.
        if (!pl) {
            return QStringLiteral("Not Available");
        }

        auto userPl = pl->powerLevelForUser(user->id());

        switch (userPl) {
        case 0:
            return QStringLiteral("Member");
        case 50:
            return QStringLiteral("Moderator");
        case 100:
            return QStringLiteral("Admin");
        default:
            return QStringLiteral("Custom");
        }
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

void UserListModel::memberAdded(QString memberId)
{
    if (m_currentRoom->memberIds().contains(memberId)) {
        return;
    }
    beginInsertRows(QModelIndex(), m_currentRoom->memberIds().size(), m_currentRoom->memberIds().size());
    endInsertRows();
}

void UserListModel::memberUpdated(QString memberId)
{
    if (!m_currentRoom->memberIds().contains(memberId)) {
        return;
    }
    auto row = m_currentRoom->memberIds().indexOf(memberId);
    if (row >= 0 && row < m_currentRoom->memberIds().size()) {
        Q_EMIT dataChanged(index(row), index(row));
    }
}

void UserListModel::memberRemoved(QString memberId)
{
    if (!m_currentRoom->memberIds().contains(memberId)) {
        return;
    }
    auto row = m_currentRoom->memberIds().indexOf(memberId);
    if (row >= 0 && row < m_currentRoom->memberIds().size()) {
        beginRemoveRows(QModelIndex(), row, row);
        endRemoveRows();
    }
}

void UserListModel::refreshAllUsers()
{
    beginResetModel();
    m_members.clear();
    m_members = m_currentRoom->members();

    connect(m_currentRoom->connection(), &Connection::loggedOut, this, [this]() {
        setRoom(nullptr);
    });
    endResetModel();
    Q_EMIT usersRefreshed();
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
