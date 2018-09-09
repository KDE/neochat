#include "userlistmodel.h"

#include <QElapsedTimer>
#include <QtCore/QDebug>
#include <QtGui/QPixmap>

#include <connection.h>
#include <room.h>
#include <user.h>

UserListModel::UserListModel(QObject* parent)
    : QAbstractListModel(parent), m_currentRoom(nullptr) {}

void UserListModel::setRoom(QMatrixClient::Room* room) {
  if (m_currentRoom == room) return;

  using namespace QMatrixClient;
  beginResetModel();
  if (m_currentRoom) {
    m_currentRoom->connection()->disconnect(this);
    m_currentRoom->disconnect(this);
    for (User* user : m_users) user->disconnect(this);
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
      QElapsedTimer et;
      et.start();
      m_users = m_currentRoom->users();
      std::sort(m_users.begin(), m_users.end(), room->memberSorter());
      qDebug() << "Sorting" << m_users.size() << "user(s) in"
               << m_currentRoom->displayName() << "took" << et;
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

QMatrixClient::User* UserListModel::userAt(QModelIndex index) {
  if (index.row() < 0 || index.row() >= m_users.size()) return nullptr;
  return m_users.at(index.row());
}

QVariant UserListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();

  if (index.row() >= m_users.count()) {
    qDebug()
        << "UserListModel, something's wrong: index.row() >= m_users.count()";
    return QVariant();
  }
  auto user = m_users.at(index.row());
  if (role == NameRole) {
    return user->displayname(m_currentRoom);
  }
  if (role == AvatarRole) {
    if (!user->avatarUrl(m_currentRoom).isEmpty())
      return user->avatar(32, m_currentRoom);
    return QImage();
  }

  return QVariant();
}

int UserListModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) return 0;

  return m_users.count();
}

void UserListModel::userAdded(QMatrixClient::User* user) {
  auto pos = findUserPos(user);
  beginInsertRows(QModelIndex(), pos, pos);
  m_users.insert(pos, user);
  endInsertRows();
  connect(user, &QMatrixClient::User::avatarChanged, this,
          &UserListModel::avatarChanged);
}

void UserListModel::userRemoved(QMatrixClient::User* user) {
  auto pos = findUserPos(user);
  if (pos != m_users.size()) {
    beginRemoveRows(QModelIndex(), pos, pos);
    m_users.removeAt(pos);
    endRemoveRows();
    user->disconnect(this);
  } else
    qWarning() << "Trying to remove a room member not in the user list";
}

void UserListModel::refresh(QMatrixClient::User* user, QVector<int> roles) {
  auto pos = findUserPos(user);
  if (pos != m_users.size())
    emit dataChanged(index(pos), index(pos), roles);
  else
    qWarning() << "Trying to access a room member not in the user list";
}

void UserListModel::avatarChanged(QMatrixClient::User* user,
                                  const QMatrixClient::Room* context) {
  if (context == m_currentRoom) refresh(user, {AvatarRole});
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
  roles[AvatarRole] = "avatar";
  return roles;
}
