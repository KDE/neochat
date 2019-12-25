#include "publicroomlistmodel.h"

PublicRoomListModel::PublicRoomListModel(QObject* parent)
    : QAbstractListModel(parent) {}

void PublicRoomListModel::setConnection(Connection* conn) {
  if (m_connection == conn)
    return;

  beginResetModel();

  nextBatch = "";
  attempted = false;
  rooms.clear();
  m_server.clear();

  if (m_connection) {
    m_connection->disconnect(this);
  }

  endResetModel();

  m_connection = conn;

  if (job) {
    job->abandon();
    job = nullptr;
  }

  if (m_connection) {
    next();
  }

  emit connectionChanged();
  emit serverChanged();
  emit hasMoreChanged();
}

void PublicRoomListModel::setServer(const QString& value) {
  if (m_server == value)
    return;

  m_server = value;

  beginResetModel();

  nextBatch = "";
  attempted = false;
  rooms.clear();

  endResetModel();

  if (job) {
    job->abandon();
    job = nullptr;
  }

  if (m_connection) {
    next();
  }

  emit serverChanged();
  emit hasMoreChanged();
}

void PublicRoomListModel::setKeyword(const QString& value) {
  if (m_keyword == value)
    return;

  m_keyword = value;

  beginResetModel();

  nextBatch = "";
  attempted = false;
  rooms.clear();

  endResetModel();

  if (job) {
    job->abandon();
    job = nullptr;
  }

  if (m_connection) {
    next();
  }

  emit keywordChanged();
  emit hasMoreChanged();
}

void PublicRoomListModel::next(int count) {
  if (count < 1)
    return;

  if (job) {
    qDebug() << "PublicRoomListModel: Other jobs running, ignore";

    return;
  }

  if (!hasMore())
    return;

  job = m_connection->callApi<QueryPublicRoomsJob>(
      m_server, count, nextBatch, QueryPublicRoomsJob::Filter{m_keyword});

  connect(job, &BaseJob::finished, this, [=] {
    attempted = true;

    if (job->status() == BaseJob::Success) {
      auto resp = job->data();
      nextBatch = resp.nextBatch;

      this->beginInsertRows({}, rooms.count(),
                            rooms.count() + resp.chunk.count() - 1);
      rooms.append(resp.chunk);
      this->endInsertRows();

      if (resp.nextBatch.isEmpty()) {
        emit hasMoreChanged();
      }
    }

    this->job = nullptr;
  });
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
    auto displayName = room.name;
    if (!displayName.isEmpty()) {
      return displayName;
    }

    displayName = room.canonicalAlias;
    if (!displayName.isEmpty()) {
      return displayName;
    }

    displayName = room.aliases.front();
    if (!displayName.isEmpty()) {
      return displayName;
    }

    return room.roomId;
  }
  if (role == AvatarRole) {
    auto avatarUrl = room.avatarUrl;

    if (avatarUrl.isEmpty()) {
      return "";
    }

    return avatarUrl.remove(0, 6);
  }
  if (role == TopicRole) {
    return room.topic;
  }

  return {};
}

QHash<int, QByteArray> PublicRoomListModel::roleNames() const {
  QHash<int, QByteArray> roles;

  roles[NameRole] = "name";
  roles[AvatarRole] = "avatar";
  roles[TopicRole] = "topic";

  return roles;
}

int PublicRoomListModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid())
    return 0;

  return rooms.count();
}

bool PublicRoomListModel::hasMore() const {
  return !(attempted && nextBatch.isEmpty());
}
