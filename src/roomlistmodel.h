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
  enum EventRoles {
    NameRole = Qt::UserRole + 1,
    AvatarRole,
    TopicRole,
    CategoryRole,
    HighlightRole,
  };

  RoomListModel(QObject* parent = 0);
  virtual ~RoomListModel();

  QMatrixClient::Connection* getConnection() { return m_connection; }
  void setConnection(QMatrixClient::Connection* connection);
  void doResetModel();

  Q_INVOKABLE QMatrixClient::Room* roomAt(int row);

  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  Q_INVOKABLE int rowCount(
      const QModelIndex& parent = QModelIndex()) const override;

  QHash<int, QByteArray> roleNames() const;

 private slots:
  void namesChanged(QMatrixClient::Room* room);
  void unreadMessagesChanged(QMatrixClient::Room* room);

  void doAddRoom(QMatrixClient::Room* room);
  void updateRoom(QMatrixClient::Room* room, QMatrixClient::Room* prev);
  void deleteRoom(QMatrixClient::Room* room);
  void refresh(QMatrixClient::Room* room, const QVector<int>& roles = {});

 private:
  QMatrixClient::Connection* m_connection = nullptr;
  QList<QMatrixClient::Room*> m_rooms;
  void connectRoomSignals(QMatrixClient::Room* room);

 signals:
  void connectionChanged();
  void highlightCountChanged(QMatrixClient::Room* room);
};

#endif  // ROOMLISTMODEL_H
