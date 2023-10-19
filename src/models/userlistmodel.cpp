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
        connect(m_currentRoom, &Room::userAdded, this, &UserListModel::userAdded);
        connect(m_currentRoom, &Room::userRemoved, this, &UserListModel::userRemoved);
        connect(m_currentRoom, &Room::memberAboutToRename, this, &UserListModel::userRemoved);
        connect(m_currentRoom, &Room::memberRenamed, this, &UserListModel::userAdded);
        connect(m_currentRoom, &Room::changed, this, &UserListModel::refreshAllUsers);
    }

    refreshAllUsers();
    Q_EMIT roomChanged();
}

NeoChatRoom *UserListModel::room() const
{
    return m_currentRoom;
}

Quotient::User *UserListModel::userAt(QModelIndex index) const
{
    if (index.row() < 0 || index.row() >= m_users.size()) {
        return nullptr;
    }
    return m_users.at(index.row());
}

QVariant UserListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_users.count()) {
        qDebug() << "UserListModel, something's wrong: index.row() >= "
                    "users.count()";
        return {};
    }
    auto user = m_users.at(index.row());
    if (role == DisplayNameRole) {
        return user->displayname(m_currentRoom);
    }
    if (role == UserIdRole) {
        return user->id();
    }
    if (role == AvatarRole) {
        return m_currentRoom->avatarForMember(user);
    }
    if (role == ObjectRole) {
        return QVariant::fromValue(user);
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
    return m_users.count();
}

void UserListModel::userAdded(Quotient::User *user)
{
    auto pos = findUserPos(user);
    beginInsertRows(QModelIndex(), pos, pos);
    m_users.insert(pos, user);
    endInsertRows();
    connect(user, &User::defaultAvatarChanged, this, [this, user]() {
        refreshUser(user, {AvatarRole});
    });
}

void UserListModel::userRemoved(Quotient::User *user)
{
    auto pos = findUserPos(user);
    if (pos != m_users.size()) {
        beginRemoveRows(QModelIndex(), pos, pos);
        m_users.removeAt(pos);
        endRemoveRows();
        user->disconnect(this);
    } else {
        qWarning() << "Trying to remove a room member not in the user list";
    }
}

void UserListModel::refreshUser(Quotient::User *user, const QList<int> &roles)
{
    auto pos = findUserPos(user);
    if (pos != m_users.size()) {
        Q_EMIT dataChanged(index(pos), index(pos), roles);
    } else {
        qWarning() << "Trying to access a room member not in the user list";
    }
}

void UserListModel::refreshAllUsers()
{
    beginResetModel();
    for (User *user : std::as_const(m_users)) {
        user->disconnect(this);
    }
    m_users.clear();

    if (m_currentRoom != nullptr) {
        m_users = m_currentRoom->users();
        std::sort(m_users.begin(), m_users.end(), m_currentRoom->memberSorter());

        for (User *user : std::as_const(m_users)) {
            connect(user, &User::defaultAvatarChanged, this, [this, user]() {
                refreshUser(user, {AvatarRole});
            });
        }
        connect(m_currentRoom->connection(), &Connection::loggedOut, this, [this]() {
            setRoom(nullptr);
        });
    }
    endResetModel();
    Q_EMIT usersRefreshed();
}

int UserListModel::findUserPos(Quotient::User *user) const
{
    return findUserPos(m_currentRoom->safeMemberName(user->id()));
}

int UserListModel::findUserPos(const QString &username) const
{
    if (!m_currentRoom) {
        return 0;
    }
    return m_currentRoom->memberSorter().lowerBoundIndex(m_users, username);
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
