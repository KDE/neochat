#ifndef PUBLICROOMLISTMODEL_H
#define PUBLICROOMLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>

#include "connection.h"
#include "csapi/definitions/public_rooms_response.h"

using namespace Quotient;

class PublicRoomListModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(Connection* connection READ connection WRITE setConnection NOTIFY connectionChanged)

 public:
  enum EventRoles { NameRole = Qt::DisplayRole + 1, TopicRole };

  PublicRoomListModel(QObject* parent = nullptr);

  QVariant data(const QModelIndex& index, int role = NameRole) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  QHash<int, QByteArray> roleNames() const override;

  Connection* connection() const { return m_connection; }
  void setConnection(Connection* value);

  Q_INVOKABLE void next(int count = 50);

 private:
  Connection* m_connection = nullptr;

  bool attempted = false;
  QString nextBatch;

  QVector<PublicRoomsChunk> rooms;

signals:
  void connectionChanged();
};

#endif // PUBLICROOMLISTMODEL_H
