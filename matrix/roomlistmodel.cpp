#include <QtGui/QBrush>
#include <QtGui/QColor>

#include "roomlistmodel.h"

#include "controller.h"

RoomListModel::RoomListModel(QMatrixClient::Connection* m_connection) : m_connection(m_connection) {
    beginResetModel();
    m_rooms.clear();
    connect(m_connection, &QMatrixClient::Connection::newRoom, this, &RoomListModel::addRoom);
    for(QMatrixClient::Room* room: m_connection->roomMap().values()) {
        connect(room, &QMatrixClient::Room::namesChanged, this, &RoomListModel::namesChanged);
        m_rooms.append(room);
    }
    endResetModel();
}

RoomListModel::~RoomListModel() {

}

QMatrixClient::Room* RoomListModel::roomAt(int row) {
    return m_rooms.at(row);
}

void RoomListModel::addRoom(QMatrixClient::Room* room) {
    qDebug() << "Adding room.";
    beginInsertRows(QModelIndex(), m_rooms.count(), m_rooms.count());
    connect(room, &QMatrixClient::Room::namesChanged, this, &RoomListModel::namesChanged );
    m_rooms.append(room);
    endInsertRows();
}

int RoomListModel::rowCount(const QModelIndex& parent) const {
    if( parent.isValid() )
        return 0;
    return m_rooms.count();
}

QVariant RoomListModel::data(const QModelIndex& index, int role) const {
    if(!index.isValid())
        return QVariant();

    if(index.row() >= m_rooms.count()) {
        qDebug() << "UserListModel: something wrong here...";
        return QVariant();
    }
    QMatrixClient::Room* room = m_rooms.at(index.row());
    if(role == NameRole) {
        return room->displayName();
    }
    if(role == ValueRole) {
        return room->topic();
    }
    if(role == AvatarRole) {
        return room->avatarUrl();
    }
    return QVariant();
}

QHash<int, QByteArray> RoomListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[ValueRole] = "value";
    roles[AvatarRole] = "avatar";
    return roles;
}

void RoomListModel::namesChanged(QMatrixClient::Room* room) {
    int row = m_rooms.indexOf(room);
    emit dataChanged(index(row), index(row));
}

void RoomListModel::unreadMessagesChanged(QMatrixClient::Room* room) {

}
