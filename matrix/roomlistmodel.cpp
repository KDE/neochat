#include <QtGui/QBrush>
#include <QtGui/QColor>

#include "roomlistmodel.h"
#include "controller.h"
#include "user.h"

RoomListModel::RoomListModel() {

}

RoomListModel::~RoomListModel() {

}

void RoomListModel::setConnection(QMatrixClient::Connection *conn) {
    m_connection = conn;
    beginResetModel();
    m_rooms.clear();
    connect(m_connection, &QMatrixClient::Connection::newRoom, this, &RoomListModel::addRoom);
    for(QMatrixClient::Room* r: m_connection->roomMap().values()) {
        if (auto* room = static_cast<MatriqueRoom*>(r))
        {
            connect(room, &MatriqueRoom::namesChanged, this, &RoomListModel::namesChanged);
            m_rooms.append(room);
        } else
        {
            qCritical() << "Attempt to add nullptr to the room list";
            Q_ASSERT(false);
        }
    }
    endResetModel();
}

MatriqueRoom* RoomListModel::roomAt(int row) {
    return m_rooms.at(row);
}

void RoomListModel::addRoom(MatriqueRoom* room) {
    qDebug() << "Adding room.";
    beginInsertRows(QModelIndex(), m_rooms.count(), m_rooms.count());
    connect(room, &MatriqueRoom::namesChanged, this, &RoomListModel::namesChanged );
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
    MatriqueRoom* room = m_rooms.at(index.row());
    if(role == NameRole) {
        return room->displayName();
    }
    if(role == ValueRole) {
        return room->topic();
    }
    if(role == AvatarRole) {
        if(room->avatarUrl().toString() != "") {
            return room->avatarUrl();
        } else if(room->users().length() == 2) {
            QMatrixClient::User* user = room->users().at(0);
            return user->avatarUrl();
        }
    }
    return QVariant();
}

QModelIndex RoomListModel::indexOf(MatriqueRoom* room) const
{
    return index(m_rooms.indexOf(room), 0);
}

QHash<int, QByteArray> RoomListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[ValueRole] = "value";
    roles[AvatarRole] = "avatar";
    return roles;
}

void RoomListModel::namesChanged(MatriqueRoom* room) {
    int row = m_rooms.indexOf(room);
    emit dataChanged(index(row), index(row));
}

void RoomListModel::unreadMessagesChanged(MatriqueRoom* room) {

}
