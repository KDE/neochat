#ifndef ROOMLISTMODEL_H
#define ROOMLISTMODEL_H

#include <QObject>
#include <QtCore/QAbstractListModel>

#include "libqmatrixclient/connection.h"
#include "libqmatrixclient/room.h"

#include "matriqueroom.h"

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

        Q_INVOKABLE MatriqueRoom* roomAt(int row);

        QVariant data(const QModelIndex& index, int role) const override;
        QModelIndex indexOf(MatriqueRoom* room) const;
        Q_INVOKABLE int rowCount(const QModelIndex& parent=QModelIndex()) const override;

    signals:
        void connectionChanged();

    public slots:

    private slots:
        void namesChanged(MatriqueRoom* room);
        void unreadMessagesChanged(MatriqueRoom* room);
        void addRoom(MatriqueRoom* room);

    private:
        QList<MatriqueRoom*> m_rooms;
};

#endif // ROOMLISTMODEL_H
