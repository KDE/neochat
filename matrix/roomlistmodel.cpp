#include "roomlistmodel.h"

#include "controller.h"

RoomListModel::RoomListModel(QObject *parent) : QObject(parent)
{

}

void RoomListModel::init(QMatrixClient::Connection *conn) {
    qDebug() << "Registering connection.";
    m_connection = conn;
    connect(m_connection, &QMatrixClient::Connection::newRoom, this, &RoomListModel::addRoom);
    for(QMatrixClient::Room* room: m_connection->roomMap().values()) {
        connect(room, &QMatrixClient::Room::namesChanged, this, &RoomListModel::namesChanged);
        m_rooms.append(room);
    }
}

RoomListModel::~RoomListModel() {

}

QMatrixClient::Room* RoomListModel::roomAt(int row)
{
    return m_rooms.at(row);
}

void RoomListModel::addRoom(QMatrixClient::Room* room)
{
    qDebug() << "Adding room.";
    connect(room, &QMatrixClient::Room::namesChanged, this, &RoomListModel::namesChanged );
    m_rooms.append(room);
}

void RoomListModel::namesChanged(QMatrixClient::Room* room)
{

}

void RoomListModel::unreadMessagesChanged(QMatrixClient::Room* room)
{

}
