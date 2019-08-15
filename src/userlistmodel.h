#ifndef USERLISTMODEL_H
#define USERLISTMODEL_H

#include "room.h"

#include <QObject>
#include <QtCore/QAbstractListModel>

namespace Quotient {
class Connection;
class Room;
class User;
}  // namespace Quotient

class UserListModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(
      Quotient::Room* room READ room WRITE setRoom NOTIFY roomChanged)
 public:
  enum EventRoles {
    NameRole = Qt::UserRole + 1,
    UserIDRole,
    AvatarRole,
    ObjectRole
  };

  using User = Quotient::User;

  UserListModel(QObject* parent = nullptr);

  Quotient::Room* room() const { return m_currentRoom; }
  void setRoom(Quotient::Room* room);
  User* userAt(QModelIndex index) const;

  QVariant data(const QModelIndex& index, int role = NameRole) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  QHash<int, QByteArray> roleNames() const override;

 signals:
  void roomChanged();

 private slots:
  void userAdded(User* user);
  void userRemoved(User* user);
  void refresh(User* user, QVector<int> roles = {});
  void avatarChanged(User* user, const Quotient::Room* context);

 private:
  Quotient::Room* m_currentRoom;
  QList<User*> m_users;

  int findUserPos(User* user) const;
  int findUserPos(const QString& username) const;
};

#endif  // USERLISTMODEL_H
