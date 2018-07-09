#ifndef ROOMLISTMODEL_H
#define ROOMLISTMODEL_H

#include <QtCore/QAbstractListModel>
#include "connection.h"
#include "room.h"

class RoomListModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(QMatrixClient::Connection* connection READ getConnection WRITE
                 setConnection)

 public:
  RoomListModel(QObject* parent = 0);
  virtual ~RoomListModel();

  QMatrixClient::Connection* getConnection() { return m_connection; }
  void setConnection(QMatrixClient::Connection* connection);

  Q_INVOKABLE QMatrixClient::Room* roomAt(int row);

  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  Q_INVOKABLE int rowCount(
      const QModelIndex& parent = QModelIndex()) const override;

  QHash<int, QByteArray> roleNames() const;

 private slots:
  void namesChanged(QMatrixClient::Room* room);
  void unreadMessagesChanged(QMatrixClient::Room* room);
  void addRoom(QMatrixClient::Room* room);
  void deleteRoom(QMatrixClient::Room* room);

 private:
  QMatrixClient::Connection* m_connection = nullptr;
  QList<QMatrixClient::Room*> m_rooms;

 signals:
  void connectionChanged();
};

#endif  // ROOMLISTMODEL_H
