#include "roomlistmodel.h"

#include <QtCore/QDebug>
#include <QtGui/QBrush>
#include <QtGui/QColor>

RoomListModel::RoomListModel(QObject* parent) : QAbstractListModel(parent) {}

RoomListModel::~RoomListModel() {}

void RoomListModel::setConnection(QMatrixClient::Connection* connection) {
  Q_ASSERT(connection);

  using QMatrixClient::Connection;
  using QMatrixClient::Room;
  beginResetModel();
  m_connection = connection;
  connect(connection, &Connection::loggedOut, this,
          [=] { setConnection(connection); });
  //  connect(connection, &Connection::invitedRoom, this,
  //          &RoomListModel::updateRoom);
  //  connect(connection, &Connection::joinedRoom, this,
  //          &RoomListModel::updateRoom);
  //  connect(connection, &Connection::leftRoom, this,
  //  &RoomListModel::updateRoom);
  connect(connection, &Connection::aboutToDeleteRoom, this,
          &RoomListModel::deleteRoom);

  for (auto r : connection->roomMap()) addRoom(r);
  endResetModel();
}

QMatrixClient::Room* RoomListModel::roomAt(int row) { return m_rooms.at(row); }

void RoomListModel::addRoom(QMatrixClient::Room* room) {
  beginInsertRows(QModelIndex(), m_rooms.count(), m_rooms.count());
  connect(room, &QMatrixClient::Room::namesChanged, this,
          &RoomListModel::namesChanged);
  m_rooms.append(room);
  endInsertRows();
}

void RoomListModel::deleteRoom(QMatrixClient::Room* room) {
  const auto it = std::find(m_rooms.begin(), m_rooms.end(), room);
  if (it == m_rooms.end()) return;  // Already deleted, nothing to do
  const int row = it - m_rooms.begin();
  beginRemoveRows(QModelIndex(), row, row);
  m_rooms.erase(it);
  endRemoveRows();
}

int RoomListModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) return 0;
  return m_rooms.count();
}

QVariant RoomListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();

  if (index.row() >= m_rooms.count()) {
    qDebug() << "UserListModel: something wrong here...";
    return QVariant();
  }
  QMatrixClient::Room* room = m_rooms.at(index.row());
  if (role == NameRole) {
    return room->displayName();
  }
  if (role == AvatarRole) {
    if (room->avatarUrl().toString() != "") {
      return room->avatarUrl();
    }
    return QVariant();
  }
  if (role == TopicRole) {
    return room->topic();
  }
  if (role == CategoryRole) {
    if (room->isFavourite()) return "Favorites";
    if (room->isLowPriority()) return "Low Priorities";
    return "Rooms";
  }
  if (role == HighlightRole) {
    if (room->highlightCount() > 0) return QBrush(QColor("orange"));
    return QVariant();
  }
  return QVariant();
}

void RoomListModel::namesChanged(QMatrixClient::Room* room) {
  int row = m_rooms.indexOf(room);
  emit dataChanged(index(row), index(row));
}

void RoomListModel::unreadMessagesChanged(QMatrixClient::Room* room) {
  int row = m_rooms.indexOf(room);
  emit dataChanged(index(row), index(row));
}

QHash<int, QByteArray> RoomListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[NameRole] = "name";
  roles[AvatarRole] = "avatar";
  roles[TopicRole] = "topic";
  roles[CategoryRole] = "category";
  roles[HighlightRole] = "highlight";
  return roles;
}
