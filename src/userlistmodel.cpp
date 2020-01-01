#include "userlistmodel.h"

#include <connection.h>
#include <room.h>
#include <user.h>

#include "events/roompowerlevelsevent.h"

#include <QElapsedTimer>
#include <QtCore/QDebug>
#include <QtGui/QPixmap>

#include "spectraluser.h"

UserListModel::UserListModel(QObject* parent)
    : QAbstractListModel(parent), m_currentRoom(nullptr) {}

void UserListModel::setRoom(Quotient::Room* room) {
  if (m_currentRoom == room)
    return;

  using namespace Quotient;
  beginResetModel();
  if (m_currentRoom) {
    m_currentRoom->disconnect(this);
    //    m_currentRoom->connection()->disconnect(this);
    for (User* user : m_users)
      user->disconnect(this);
    m_users.clear();
  }
  m_currentRoom = room;
  if (m_currentRoom) {
    connect(m_currentRoom, &Room::userAdded, this, &UserListModel::userAdded);
    connect(m_currentRoom, &Room::userRemoved, this,
            &UserListModel::userRemoved);
    connect(m_currentRoom, &Room::memberAboutToRename, this,
            &UserListModel::userRemoved);
    connect(m_currentRoom, &Room::memberRenamed, this,
            &UserListModel::userAdded);
    {
      m_users = m_currentRoom->users();
      std::sort(m_users.begin(), m_users.end(), room->memberSorter());
    }
    for (User* user : m_users) {
      connect(user, &User::avatarChanged, this, &UserListModel::avatarChanged);
    }
    connect(m_currentRoom->connection(), &Connection::loggedOut, this,
            [=] { setRoom(nullptr); });
    qDebug() << m_users.count() << "user(s) in the room";
  }
  endResetModel();
  emit roomChanged();
}

Quotient::User* UserListModel::userAt(QModelIndex index) const {
  if (index.row() < 0 || index.row() >= m_users.size())
    return nullptr;
  return m_users.at(index.row());
}

QVariant UserListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid())
    return QVariant();

  if (index.row() >= m_users.count()) {
    qDebug()
        << "UserListModel, something's wrong: index.row() >= m_users.count()";
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

    if (userPl == pl->content().usersDefault) {
      return UserType::Member;
    }

    if (userPl < pl->content().usersDefault) {
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
      return UserType::Admin;
    }

    return UserType::Moderator;
  }

  return {};
}

int UserListModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid())
    return 0;

  return m_users.count();
}

void UserListModel::userAdded(Quotient::User* user) {
  auto pos = findUserPos(user);
  beginInsertRows(QModelIndex(), pos, pos);
  m_users.insert(pos, user);
  endInsertRows();
  connect(user, &Quotient::User::avatarChanged, this,
          &UserListModel::avatarChanged);
}

void UserListModel::userRemoved(Quotient::User* user) {
  auto pos = findUserPos(user);
  if (pos != m_users.size()) {
    beginRemoveRows(QModelIndex(), pos, pos);
    m_users.removeAt(pos);
    endRemoveRows();
    user->disconnect(this);
  } else
    qWarning() << "Trying to remove a room member not in the user list";
}

void UserListModel::refresh(Quotient::User* user, QVector<int> roles) {
  auto pos = findUserPos(user);
  if (pos != m_users.size())
    emit dataChanged(index(pos), index(pos), roles);
  else
    qWarning() << "Trying to access a room member not in the user list";
}

void UserListModel::avatarChanged(Quotient::User* user,
                                  const Quotient::Room* context) {
  if (context == m_currentRoom)
    refresh(user, {AvatarRole});
}

int UserListModel::findUserPos(User* user) const {
  return findUserPos(m_currentRoom->roomMembername(user));
}

int UserListModel::findUserPos(const QString& username) const {
  return m_currentRoom->memberSorter().lowerBoundIndex(m_users, username);
}

QHash<int, QByteArray> UserListModel::roleNames() const {
  QHash<int, QByteArray> roles;

  roles[NameRole] = "name";
  roles[UserIDRole] = "userId";
  roles[AvatarRole] = "avatar";
  roles[ObjectRole] = "user";
  roles[PermRole] = "perm";

  return roles;
}
