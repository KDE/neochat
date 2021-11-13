// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "userlistmodel.h"

#include <connection.h>
#include <room.h>
#include <user.h>

#include "events/roompowerlevelsevent.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QPixmap>

#include "neochatuser.h"

UserListModel::UserListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_currentRoom(nullptr)
{
}

void UserListModel::setRoom(Quotient::Room *room)
{
    if (m_currentRoom == room) {
        return;
    }

    using namespace Quotient;
    beginResetModel();
    if (m_currentRoom) {
        m_currentRoom->disconnect(this);
        //    m_currentRoom->connection()->disconnect(this);
        for (User *user : std::as_const(m_users)) {
            user->disconnect(this);
        }
        m_users.clear();
    }
    m_currentRoom = room;
    if (m_currentRoom) {
        connect(m_currentRoom, &Room::userAdded, this, &UserListModel::userAdded);
        connect(m_currentRoom, &Room::userRemoved, this, &UserListModel::userRemoved);
        connect(m_currentRoom, &Room::memberAboutToRename, this, &UserListModel::userRemoved);
        connect(m_currentRoom, &Room::memberRenamed, this, &UserListModel::userAdded);
        {
            m_users = m_currentRoom->users();
            std::sort(m_users.begin(), m_users.end(), room->memberSorter());
        }
        for (User *user : std::as_const(m_users)) {
#ifdef QUOTIENT_07
            connect(user, &User::defaultAvatarChanged, this, [this, user]() {
                avatarChanged(user, m_currentRoom);
            });
#else
            connect(user, &User::avatarChanged, this, &UserListModel::avatarChanged);
#endif
        }
        connect(m_currentRoom->connection(), &Connection::loggedOut, this, [this] {
            setRoom(nullptr);
        });
        qDebug() << m_users.count() << "user(s) in the room";
    }
    endResetModel();
    Q_EMIT roomChanged();
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
        qDebug() << "UserListModel, something's wrong: index.row() >= m_users.count()";
        return {};
    }
    auto user = m_users.at(index.row());
    if (role == NameRole) {
        return user->displayname(m_currentRoom);
    }
    if (role == UserIDRole) {
        return user->id();
    }
    if (role == AvatarRole) {
        return user->avatarMediaId(m_currentRoom);
    }
    if (role == ObjectRole) {
        return QVariant::fromValue(user);
    }
    if (role == PermRole) {
        auto pl = m_currentRoom->getCurrentState<RoomPowerLevelsEvent>();
        auto userPl = pl->powerLevelForUser(user->id());

        if (userPl == pl->content().usersDefault) { // Shortcut
            return UserType::Member;
        }

        if (userPl < pl->powerLevelForEvent("m.room.message")) {
            return UserType::Muted;
        }

        auto userPls = pl->users();

        int highestPl = pl->usersDefault();
        QHash<QString, int>::const_iterator i = userPls.constBegin();
        while (i != userPls.constEnd()) {
            if (i.value() > highestPl) {
                highestPl = i.value();
            }

            ++i;
        }

        if (userPl == highestPl) {
            return UserType::Owner;
        }

        if (userPl >= pl->powerLevelForState("m.room.power_levels")) {
            return UserType::Admin;
        }

        if (userPl >= pl->ban() || userPl >= pl->kick() || userPl >= pl->redact()) {
            return UserType::Moderator;
        }

        return UserType::Member;
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
#ifdef QUOTIENT_07
    connect(user, &User::defaultAvatarChanged, this, [this, user]() {
        avatarChanged(user, m_currentRoom);
    });
#else
    connect(user, &Quotient::User::avatarChanged, this, &UserListModel::avatarChanged);
#endif
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

void UserListModel::refresh(Quotient::User *user, const QVector<int> &roles)
{
    auto pos = findUserPos(user);
    if (pos != m_users.size()) {
        Q_EMIT dataChanged(index(pos), index(pos), roles);
    } else {
        qWarning() << "Trying to access a room member not in the user list";
    }
}

void UserListModel::avatarChanged(Quotient::User *user, const Quotient::Room *context)
{
    if (context == m_currentRoom) {
        refresh(user, {AvatarRole});
    }
}

int UserListModel::findUserPos(User *user) const
{
    return findUserPos(m_currentRoom->roomMembername(user));
}

int UserListModel::findUserPos(const QString &username) const
{
    return m_currentRoom->memberSorter().lowerBoundIndex(m_users, username);
}

QHash<int, QByteArray> UserListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[NameRole] = "name";
    roles[UserIDRole] = "userId";
    roles[AvatarRole] = "avatar";
    roles[ObjectRole] = "user";
    roles[PermRole] = "perm";

    return roles;
}
