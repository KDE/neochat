#include "publicroomlistmodel.h"

#include "csapi/list_public_rooms.h"

PublicRoomListModel::PublicRoomListModel(QObject* parent)
    : QAbstractListModel(parent) {}

void PublicRoomListModel::setConnection(Connection* conn) {
  if (m_connection == conn)
    return;

  beginResetModel();

  nextBatch = "";
  attempted = false;
  rooms.clear();

  if (m_connection) {
    m_connection->disconnect(this);
  }

  endResetModel();

  m_connection = conn;

  if (m_connection) {
    next();
  }

  emit connectionChanged();
}

void PublicRoomListModel::next(int count) {
  if (count < 1)
    return;

  if (attempted && nextBatch.isEmpty())
    return;

  auto job = m_connection->callApi<GetPublicRoomsJob>(count, nextBatch);

  connect(job, &BaseJob::success, this, [=] {
    auto resp = job->data();
    nextBatch = resp.nextBatch;

    this->beginInsertRows({}, rooms.count(),
                          rooms.count() + resp.chunk.count());
    rooms.append(resp.chunk);
    this->endInsertRows();
  });

  connect(job, &BaseJob::finished, this, [=] { attempted = true; });
}

QVariant PublicRoomListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid())
    return QVariant();

  if (index.row() >= rooms.count()) {
    qDebug() << "PublicRoomListModel, something's wrong: index.row() >= "
                "rooms.count()";
    return {};
  }
  auto room = rooms.at(index.row());
  if (role == NameRole) {
    return room.name;
  }
  if (role == TopicRole) {
    return room.topic;
  }

  return {};
}

QHash<int, QByteArray> PublicRoomListModel::roleNames() const {
  QHash<int, QByteArray> roles;

  roles[NameRole] = "name";
  roles[TopicRole] = "topic";

  return roles;
}

int PublicRoomListModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid())
    return 0;

  return rooms.count();
}
