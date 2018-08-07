#include "roomlistmodel.h"

#include "events/roomevent.h"

#include <QtCore/QDebug>
#include <QtGui/QBrush>
#include <QtGui/QColor>

RoomListModel::RoomListModel(QObject* parent) : QAbstractListModel(parent) {}

RoomListModel::~RoomListModel() {}

void RoomListModel::setConnection(Connection* connection) {
  Q_ASSERT(connection);

  using QMatrixClient::Room;
  m_connection = connection;

  if (!connection->accessToken().isEmpty()) doResetModel();

  connect(connection, &Connection::connected, this,
          &RoomListModel::doResetModel);
  connect(connection, &Connection::invitedRoom, this,
          &RoomListModel::updateRoom);
  connect(connection, &Connection::joinedRoom, this,
          &RoomListModel::updateRoom);
  connect(connection, &Connection::leftRoom, this, &RoomListModel::updateRoom);
  connect(connection, &Connection::aboutToDeleteRoom, this,
          &RoomListModel::deleteRoom);
}

void RoomListModel::doResetModel() {
  beginResetModel();
  m_rooms.clear();
  for (auto r : m_connection->roomMap()) doAddRoom(r);
  endResetModel();
}

Room* RoomListModel::roomAt(int row) { return m_rooms.at(row); }

void RoomListModel::doAddRoom(Room* r) {
  if (auto* room = r) {
    m_rooms.append(room);
    connectRoomSignals(room);
  } else {
    qCritical() << "Attempt to add nullptr to the room list";
    Q_ASSERT(false);
  }
}

void RoomListModel::connectRoomSignals(Room* room) {
  connect(room, &Room::displaynameChanged, this, [=] { namesChanged(room); });
  connect(room, &Room::unreadMessagesChanged, this,
          [=] { unreadMessagesChanged(room); });
  connect(room, &Room::notificationCountChanged, this,
          [=] { unreadMessagesChanged(room); });
  connect(room, &Room::tagsChanged, this, [=] { refresh(room); });
  connect(room, &Room::joinStateChanged, this, [=] { refresh(room); });
  connect(room, &Room::avatarChanged, this,
          [=] { refresh(room, {AvatarRole}); });

  connect(room, &Room::unreadMessagesChanged, this, [=](Room* r) {
    if (r->hasUnreadMessages()) emit newMessage(r);
  });
  //  connect(
  //      room, &QMatrixClient::Room::aboutToAddNewMessages, this,
  //      [=](QMatrixClient::RoomEventsRange eventsRange) {
  //        for (QMatrixClient::RoomEvents events : eventsRange.const_iterator)
  //        {
  //          for (QMatrixClient::RoomEvent event : events) {
  //            qDebug() << event.fullJson();
  //          }
  //        }
  //        emit newMessage(room);
  //      });
}

void RoomListModel::updateRoom(Room* room, Room* prev) {
  // There are two cases when this method is called:
  // 1. (prev == nullptr) adding a new room to the room list
  // 2. (prev != nullptr) accepting/rejecting an invitation or inviting to
  //    the previously left room (in both cases prev has the previous state).
  if (prev == room) {
    qCritical() << "RoomListModel::updateRoom: room tried to replace itself";
    refresh(static_cast<Room*>(room));
    return;
  }
  if (prev && room->id() != prev->id()) {
    qCritical() << "RoomListModel::updateRoom: attempt to update room"
                << room->id() << "to" << prev->id();
    // That doesn't look right but technically we still can do it.
  }
  // Ok, we're through with pre-checks, now for the real thing.
  auto* newRoom = room;
  const auto it =
      std::find_if(m_rooms.begin(), m_rooms.end(),
                   [=](const Room* r) { return r == prev || r == newRoom; });
  if (it != m_rooms.end()) {
    const int row = it - m_rooms.begin();
    // There's no guarantee that prev != newRoom
    if (*it == prev && *it != newRoom) {
      prev->disconnect(this);
      m_rooms.replace(row, newRoom);
      connectRoomSignals(newRoom);
    }
    emit dataChanged(index(row), index(row));
  } else {
    beginInsertRows(QModelIndex(), m_rooms.count(), m_rooms.count());
    doAddRoom(newRoom);
    endInsertRows();
  }
}

void RoomListModel::deleteRoom(Room* room) {
  qDebug() << "Deleting room" << room->id();
  const auto it = std::find(m_rooms.begin(), m_rooms.end(), room);
  if (it == m_rooms.end()) return;  // Already deleted, nothing to do
  qDebug() << "Erasing room" << room->id();
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
  Room* room = m_rooms.at(index.row());
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
    //    if (!room->isDirectChat())
    //      qDebug() << room->displayName() << "is not direct.";
    if (room->isFavourite()) return "Favorites";
    if (room->isDirectChat()) return "People";
    if (room->isLowPriority()) return "Low Priorities";
    return "Rooms";
  }
  if (role == HighlightRole) {
    if (room->highlightCount() > 0) return QBrush(QColor("orange"));
    return QVariant();
  }
  if (role == UnreadCountRole) {
    return room->unreadCount();
  }
  return QVariant();
}

void RoomListModel::namesChanged(Room* room) {
  int row = m_rooms.indexOf(room);
  emit dataChanged(index(row), index(row));
}

void RoomListModel::refresh(Room* room, const QVector<int>& roles) {
  const auto it = std::find(m_rooms.begin(), m_rooms.end(), room);
  if (it == m_rooms.end()) {
    qCritical() << "Room" << room->id() << "not found in the room list";
    return;
  }
  const auto idx = index(it - m_rooms.begin());
  emit dataChanged(idx, idx, roles);
}

void RoomListModel::unreadMessagesChanged(Room* room) {
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
  roles[UnreadCountRole] = "unreadCount";
  return roles;
}
