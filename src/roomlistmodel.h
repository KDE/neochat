#ifndef ROOMLISTMODEL_H
#define ROOMLISTMODEL_H

#include "connection.h"
#include "events/roomevent.h"
#include "room.h"
#include "spectralroom.h"

#include <QtCore/QAbstractListModel>

using namespace QMatrixClient;

class RoomType : public QObject {
  Q_OBJECT

 public:
  enum Types {
    Invited = 1,
    Favorite,
    Normal,
    Direct,
    Deprioritized,
  };
  REGISTER_ENUM(Types)
};

class RoomListModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(Connection* connection READ connection WRITE setConnection)

 public:
  enum EventRoles {
    NameRole = Qt::UserRole + 1,
    AvatarRole,
    TopicRole,
    CategoryRole,
    UnreadCountRole,
    HighlightCountRole,
    LastEventRole,
    LastActiveTimeRole,
    CurrentRoomRole,
  };

  RoomListModel(QObject* parent = 0);
  virtual ~RoomListModel();

  Connection* connection() { return m_connection; }
  void setConnection(Connection* connection);
  void doResetModel();

  Q_INVOKABLE SpectralRoom* roomAt(int row);

  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  Q_INVOKABLE int rowCount(
      const QModelIndex& parent = QModelIndex()) const override;

  QHash<int, QByteArray> roleNames() const;

 private slots:
  void namesChanged(SpectralRoom* room);
  void unreadMessagesChanged(SpectralRoom* room);

  void doAddRoom(Room* room);
  void updateRoom(Room* room, Room* prev);
  void deleteRoom(Room* room);
  void refresh(SpectralRoom* room, const QVector<int>& roles = {});

 private:
  Connection* m_connection = nullptr;
  QList<SpectralRoom*> m_rooms;
  void connectRoomSignals(SpectralRoom* room);

 signals:
  void connectionChanged();
  void roomAdded(SpectralRoom* room);
  void newMessage(const QString& roomName, const QString& content,
                  const QIcon& icon);
};

#endif  // ROOMLISTMODEL_H
