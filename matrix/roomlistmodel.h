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

    Q_PROPERTY(QMatrixClient::Connection *connection READ getConnection WRITE setConnection NOTIFY connectionChanged)

    public:
        explicit RoomListModel();
        ~RoomListModel();

        enum RoomModelRoles {
            NameRole, ValueRole, AvatarRole
        };

        QMatrixClient::Connection* m_connection;
        QMatrixClient::Connection* getConnection() { return m_connection; }
        void setConnection(QMatrixClient::Connection* conn);

        QHash<int, QByteArray> roleNames() const;

        Q_INVOKABLE QMatrixClient::Room* roomAt(int row);

        QVariant data(const QModelIndex& index, int role) const override;
        Q_INVOKABLE int rowCount(const QModelIndex& parent=QModelIndex()) const override;

    signals:
        void connectionChanged();

    public slots:

    private slots:
        void namesChanged(QMatrixClient::Room* room);
        void unreadMessagesChanged(QMatrixClient::Room* room);
        void addRoom(QMatrixClient::Room* room);

    private:
        QList<QMatrixClient::Room*> m_rooms;
};

#endif // ROOMLISTMODEL_H
