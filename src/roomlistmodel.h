#ifndef ROOMLISTMODEL_H
#define ROOMLISTMODEL_H

#include <QtCore/QAbstractListModel>
#include "connection.h"
#include "room.h"

using namespace QMatrixClient;

class RoomListModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(Connection* connection READ getConnection WRITE setConnection)

 public:
  enum EventRoles {
    NameRole = Qt::UserRole + 1,
    AvatarRole,
    TopicRole,
    CategoryRole,
    HighlightRole,
    UnreadCountRole,
  };

  RoomListModel(QObject* parent = 0);
  virtual ~RoomListModel();

  Connection* getConnection() { return m_connection; }
  void setConnection(Connection* connection);
  void doResetModel();

  Q_INVOKABLE Room* roomAt(int row);

  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  Q_INVOKABLE int rowCount(
      const QModelIndex& parent = QModelIndex()) const override;

  QHash<int, QByteArray> roleNames() const;

 private slots:
  void namesChanged(Room* room);
  void unreadMessagesChanged(Room* room);

  void doAddRoom(Room* room);
  void updateRoom(Room* room, Room* prev);
  void deleteRoom(Room* room);
  void refresh(Room* room, const QVector<int>& roles = {});

 private:
  Connection* m_connection = nullptr;
  QList<Room*> m_rooms;
  void connectRoomSignals(Room* room);

 signals:
  void connectionChanged();
  void newMessage(Room* room);
};

#endif  // ROOMLISTMODEL_H
