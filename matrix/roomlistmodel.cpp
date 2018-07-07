#include "roomlistmodel.h"

#include <QtGui/QBrush>
#include <QtGui/QColor>
#include <QtCore/QDebug>

RoomListModel::RoomListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_connection = 0;
}

RoomListModel::~RoomListModel()
{
}

void RoomListModel::setConnection(QMatrixClient::Connection* connection)
{
    beginResetModel();
    m_connection = connection;
    m_rooms.clear();
    connect( connection, &QMatrixClient::Connection::newRoom, this, &RoomListModel::addRoom );
    for( QMatrixClient::Room* room: connection->roomMap().values() ) {
        connect( room, &QMatrixClient::Room::namesChanged, this, &RoomListModel::namesChanged );
        m_rooms.append(room);
    }
    endResetModel();
}

QMatrixClient::Room* RoomListModel::roomAt(int row)
{
    return m_rooms.at(row);
}

void RoomListModel::addRoom(QMatrixClient::Room* room)
{
    beginInsertRows(QModelIndex(), m_rooms.count(), m_rooms.count());
    connect( room, &QMatrixClient::Room::namesChanged, this, &RoomListModel::namesChanged );
    m_rooms.append(room);
    endInsertRows();
}

int RoomListModel::rowCount(const QModelIndex& parent) const
{
    if( parent.isValid() )
        return 0;
    return m_rooms.count();
}

QVariant RoomListModel::data(const QModelIndex& index, int role) const
{
    if( !index.isValid() )
        return QVariant();

    if( index.row() >= m_rooms.count() )
    {
        qDebug() << "UserListModel: something wrong here...";
        return QVariant();
    }
    QMatrixClient::Room* room = m_rooms.at(index.row());
    if( role == Qt::DisplayRole )
    {
        return room->displayName();
    }
    if( role == Qt::ForegroundRole )
    {
        if( room->highlightCount() > 0 )
            return QBrush(QColor("orange"));
        return QVariant();
    }
    if( role == Qt::DecorationRole )
    {
        if ( room->avatarUrl().toString() != "" ) {
            return room->avatarUrl();
        }
        return QVariant();
    }
    if ( role == Qt::StatusTipRole )
    {
        return room->topic();
    }
    return QVariant();
}

void RoomListModel::namesChanged(QMatrixClient::Room* room)
{
    int row = m_rooms.indexOf(room);
    emit dataChanged(index(row), index(row));
}

void RoomListModel::unreadMessagesChanged(QMatrixClient::Room* room)
{
    int row = m_rooms.indexOf(room);
    emit dataChanged(index(row), index(row));
}

QHash<int, QByteArray> RoomListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "name";
    roles[Qt::DecorationRole] = "avatar";
    roles[Qt::StatusTipRole] = "topic";
    return roles;
}
