#ifndef ROOMLISTMODEL_H
#define ROOMLISTMODEL_H

#include <QObject>

#include "libqmatrixclient/connection.h"
#include "libqmatrixclient/room.h"

namespace QMatrixClient {
    class Connection;
    class Room;
}

class RoomListModel : public QObject
{
    Q_OBJECT
public:
    explicit RoomListModel(QObject *parent = nullptr);
    ~RoomListModel();

    void init(QMatrixClient::Connection*);

    Q_INVOKABLE QMatrixClient::Room* roomAt(int row);

signals:

public slots:

private slots:
    void namesChanged(QMatrixClient::Room* room);
    void unreadMessagesChanged(QMatrixClient::Room* room);
    void addRoom(QMatrixClient::Room* room);

private:
    QMatrixClient::Connection *m_connection;
    QList<QMatrixClient::Room*> m_rooms;
};

#endif // ROOMLISTMODEL_H
