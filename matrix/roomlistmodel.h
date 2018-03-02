#ifndef ROOMLISTMODEL_H
#define ROOMLISTMODEL_H

#include <QObject>
#include <QtCore/QAbstractListModel>

#include "libqmatrixclient/connection.h"
#include "libqmatrixclient/room.h"

namespace QMatrixClient {
    class Connection;
    class Room;
}

class RoomListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit RoomListModel(QMatrixClient::Connection* m_connection = 0);
    ~RoomListModel();

    enum RoomModelRoles {
        NameRole, ValueRole, AvatarRole
    };

    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE QMatrixClient::Room* roomAt(int row);

    QVariant data(const QModelIndex& index, int role) const override;
    Q_INVOKABLE int rowCount(const QModelIndex& parent=QModelIndex()) const override;

signals:

public slots:

private slots:
    void namesChanged(QMatrixClient::Room* room);
    void unreadMessagesChanged(QMatrixClient::Room* room);
    void addRoom(QMatrixClient::Room* room);

private:
    QMatrixClient::Connection* m_connection;
    QList<QMatrixClient::Room*> m_rooms;
};

#endif // ROOMLISTMODEL_H
