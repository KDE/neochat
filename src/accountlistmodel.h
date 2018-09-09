#ifndef ACCOUNTLISTMODEL_H
#define ACCOUNTLISTMODEL_H

#include "controller.h"

#include <QAbstractListModel>
#include <QObject>

class AccountListModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(Controller* controller READ controller WRITE setController NOTIFY
                 controllerChanged)
 public:
  enum EventRoles {
    NameRole = Qt::UserRole + 1,
    AccountIDRole,
    AvatarRole,
    ConnectionRole
  };

  AccountListModel(QObject* parent = nullptr);

  QVariant data(const QModelIndex& index, int role = NameRole) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  QHash<int, QByteArray> roleNames() const override;

  Controller* controller() { return m_controller; }
  void setController(Controller* value);

 private:
  Controller* m_controller;
  QVector<Connection*> m_connections;

 signals:
  void controllerChanged();
};

#endif  // ACCOUNTLISTMODEL_H
