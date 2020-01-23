#ifndef ROOMLISTMODEL_H
#define ROOMLISTMODEL_H

#include "connection.h"
#include "events/roomevent.h"
#include "room.h"
#include "spectralroom.h"

#include <QtCore/QAbstractListModel>

using namespace Quotient;

class RoomType : public QObject {
  Q_OBJECT

 public:
  enum Types {
    Invited = 1,
    Favorite,
    Direct,
    Normal,
    Deprioritized,
  };
  Q_ENUMS(Types)
};

class RoomListModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(Connection* connection READ connection WRITE setConnection)
  Q_PROPERTY(int notificationCount READ notificationCount NOTIFY
                 notificationCountChanged)

 public:
  enum EventRoles {
    NameRole = Qt::UserRole + 1,
    AvatarRole,
    TopicRole,
    CategoryRole,
    UnreadCountRole,
    NotificationCountRole,
    HighlightCountRole,
    LastEventRole,
    LastActiveTimeRole,
    JoinStateRole,
    CurrentRoomRole,
  };

  RoomListModel(QObject* parent = nullptr);
  virtual ~RoomListModel() override;

  Connection* connection() const { return m_connection; }
  void setConnection(Connection* connection);
  void doResetModel();

  Q_INVOKABLE SpectralRoom* roomAt(int row) const;

  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  Q_INVOKABLE int rowCount(
      const QModelIndex& parent = QModelIndex()) const override;

  QHash<int, QByteArray> roleNames() const override;

  int notificationCount() const { return m_notificationCount; }

 private slots:
  void doAddRoom(Room* room);
  void updateRoom(Room* room, Room* prev);
  void deleteRoom(Room* room);
  void refresh(SpectralRoom* room, const QVector<int>& roles = {});
  void refreshNotificationCount();

 private:
  Connection* m_connection = nullptr;
  QList<SpectralRoom*> m_rooms;

  int m_notificationCount = 0;

  void connectRoomSignals(SpectralRoom* room);

 signals:
  void connectionChanged();
  void notificationCountChanged();

  void roomAdded(SpectralRoom* room);
  void newMessage(const QString& roomId,
                  const QString& eventId,
                  const QString& roomName,
                  const QString& senderName,
                  const QString& text,
                  const QImage& icon);
  void newHighlight(const QString& roomId,
                  const QString& eventId,
                  const QString& roomName,
                  const QString& senderName,
                  const QString& text,
                  const QImage& icon);
};

#endif  // ROOMLISTMODEL_H
