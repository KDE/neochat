#include "roomlistmodel.h"
#include "matriqueroom.h"
#include "connection.h"
#include "user.h"

#include <QtGui/QIcon>

RoomListModel::RoomListModel(QObject* parent)
    : QAbstractListModel(parent)
{ }

void RoomListModel::setConnection(QMatrixClient::Connection* connection)
{
    Q_ASSERT(connection);

    using QMatrixClient::Room;
    beginResetModel();
    m_connection = connection;
    connect( connection, &QMatrixClient::Connection::loggedOut,
             this, [=]{ deleteConnection(connection); } );
    connect( connection, &QMatrixClient::Connection::invitedRoom,
             this, &RoomListModel::updateRoom);
    connect( connection, &QMatrixClient::Connection::joinedRoom,
             this, &RoomListModel::updateRoom);
    connect( connection, &QMatrixClient::Connection::leftRoom,
             this, &RoomListModel::updateRoom);
    connect( connection, &QMatrixClient::Connection::aboutToDeleteRoom,
             this, &RoomListModel::deleteRoom);

    for( auto r: connection->roomMap() )
        doAddRoom(r);
    endResetModel();
}

void RoomListModel::deleteConnection(QMatrixClient::Connection* connection) {

}

MatriqueRoom* RoomListModel::roomAt(QModelIndex index) const
{
    return m_rooms.at(index.row());
}

QModelIndex RoomListModel::indexOf(MatriqueRoom* room) const
{
    return index(m_rooms.indexOf(room), 0);
}

void RoomListModel::updateRoom(QMatrixClient::Room* room,
                               QMatrixClient::Room* prev)
{
    // There are two cases when this method is called:
    // 1. (prev == nullptr) adding a new room to the room list
    // 2. (prev != nullptr) accepting/rejecting an invitation or inviting to
    //    the previously left room (in both cases prev has the previous state).
    if (prev == room)
    {
        qCritical() << "RoomListModel::updateRoom: room tried to replace itself";
        refresh(static_cast<MatriqueRoom*>(room));
        return;
    }
    if (prev && room->id() != prev->id())
    {
        qCritical() << "RoomListModel::updateRoom: attempt to update room"
                    << room->id() << "to" << prev->id();
        // That doesn't look right but technically we still can do it.
    }
    // Ok, we're through with pre-checks, now for the real thing.
    auto* newRoom = static_cast<MatriqueRoom*>(room);
    const auto it = std::find_if(m_rooms.begin(), m_rooms.end(),
          [=](const MatriqueRoom* r) { return r == prev || r == newRoom; });
    if (it != m_rooms.end())
    {
        const int row = it - m_rooms.begin();
        // There's no guarantee that prev != newRoom
        if (*it == prev && *it != newRoom)
        {
            prev->disconnect(this);
            m_rooms.replace(row, newRoom);
            connectRoomSignals(newRoom);
        }
        emit dataChanged(index(row), index(row));
    }
    else
    {
        beginInsertRows(QModelIndex(), m_rooms.count(), m_rooms.count());
        doAddRoom(newRoom);
        endInsertRows();
    }
}

void RoomListModel::deleteRoom(QMatrixClient::Room* room)
{
    auto i = m_rooms.indexOf(static_cast<MatriqueRoom*>(room));
    if (i == -1)
        return; // Already deleted, nothing to do

    beginRemoveRows(QModelIndex(), i, i);
    m_rooms.removeAt(i);
    endRemoveRows();
}

void RoomListModel::doAddRoom(QMatrixClient::Room* r)
{
    if (auto* room = static_cast<MatriqueRoom*>(r))
    {
        m_rooms.append(room);
        connectRoomSignals(room);
    } else
    {
        qCritical() << "Attempt to add nullptr to the room list";
        Q_ASSERT(false);
    }
}

void RoomListModel::connectRoomSignals(MatriqueRoom* room)
{
    connect(room, &MatriqueRoom::displaynameChanged,
            this, [=]{ displaynameChanged(room); } );
    connect( room, &MatriqueRoom::unreadMessagesChanged,
             this, [=]{ unreadMessagesChanged(room); } );
    connect( room, &MatriqueRoom::notificationCountChanged,
             this, [=]{ unreadMessagesChanged(room); } );
    connect( room, &MatriqueRoom::joinStateChanged,
             this, [=]{ refresh(room); });
    connect( room, &MatriqueRoom::avatarChanged,
             this, [=]{ refresh(room, { Qt::DecorationRole }); });
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
    auto room = m_rooms.at(index.row());
    using QMatrixClient::JoinState;
    switch (role)
    {
        case Qt::DisplayRole:
            return room->displayName();
        case Qt::DecorationRole:
        {
//            auto avatar = room->avatar(16, 16);
//            if (!avatar.isNull())
//                return avatar;
//            switch( room->joinState() )
//            {
//                case JoinState::Join:
//                    return QIcon(":/irc-channel-joined.svg");
//                case JoinState::Invite:
//                    return QIcon(":/irc-channel-invited.svg");
//                case JoinState::Leave:
//                    return QIcon(":/irc-channel-parted.svg");
//            }
            if(room->avatarUrl().toString() != "") {
                return room->avatarUrl();
            } else if(room->users().length() == 2) {
                QMatrixClient::User* user = room->users().at(0);
                return user->avatarUrl();
            }
        }
        case Qt::StatusTipRole:
        {
            return room->topic();
        }
        case Qt::ToolTipRole:
        {
            int hlCount = room->highlightCount();
            auto result = QStringLiteral("<b>%1</b><br>").arg(room->displayName());
            result += tr("Main alias: %1<br>").arg(room->canonicalAlias());
            result += tr("Members: %1<br>").arg(room->memberCount());
            if (hlCount > 0)
                result += tr("Unread mentions: %1<br>").arg(hlCount);
            result += tr("ID: %1<br>").arg(room->id());
            switch (room->joinState())
            {
                case JoinState::Join:
                    result += tr("You joined this room");
                    break;
                case JoinState::Leave:
                    result += tr("You left this room");
                    break;
                default:
                    result += tr("You were invited into this room");
            }
            return result;
        }
        case HasUnreadRole:
            return room->hasUnreadMessages();
        case HighlightCountRole:
            return room->highlightCount();
        case JoinStateRole:
            return toCString(room->joinState()); // FIXME: better make the enum QVariant-convertible
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> RoomListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "name";
    roles[Qt::DecorationRole] = "avatar";
    roles[Qt::StatusTipRole] = "value";
    return roles;
}

void RoomListModel::displaynameChanged(MatriqueRoom* room)
{
    refresh(room);
}

void RoomListModel::unreadMessagesChanged(MatriqueRoom* room)
{
    refresh(room);
}

void RoomListModel::refresh(MatriqueRoom* room, const QVector<int>& roles)
{
    int row = m_rooms.indexOf(room);
    if (row == -1)
        qCritical() << "Room" << room->id() << "not found in the room list";
    else
        emit dataChanged(index(row), index(row), roles);
}
