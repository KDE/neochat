#ifndef ROOMLISTMODEL_H
#define ROOMLISTMODEL_H

#include <QtCore/QAbstractListModel>

#include "matriqueroom.h"

namespace QMatrixClient
{
    class Connection;
    class Room;
}

class RoomListModel: public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QMatrixClient::Connection *connection READ getConnection WRITE setConnection)

    public:
        enum Roles {
            HasUnreadRole = Qt::UserRole + 1,
            HighlightCountRole, JoinStateRole
        };

        explicit RoomListModel(QObject* parent = nullptr);

        QMatrixClient::Connection* getConnection() { return m_connection; }
        void setConnection(QMatrixClient::Connection* connection);
        void deleteConnection(QMatrixClient::Connection* connection);

        MatriqueRoom* roomAt(QModelIndex index) const;
        QModelIndex indexOf(MatriqueRoom* room) const;

        QVariant data(const QModelIndex& index, int role) const override;
        QHash<int, QByteArray> roleNames() const;
        int rowCount(const QModelIndex& parent) const override;

    private slots:
        void displaynameChanged(MatriqueRoom* room);
        void unreadMessagesChanged(MatriqueRoom* room);
        void refresh(MatriqueRoom* room, const QVector<int>& roles = {});

        void updateRoom(QMatrixClient::Room* room,
                        QMatrixClient::Room* prev);
        void deleteRoom(QMatrixClient::Room* room);

    private:
        QMatrixClient::Connection* m_connection;
        QList<MatriqueRoom*> m_rooms;

        void doAddRoom(QMatrixClient::Room* r);
        void connectRoomSignals(MatriqueRoom* room);
};

#endif // ROOMLISTMODEL_H
