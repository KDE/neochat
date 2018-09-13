#ifndef MATRIQUEUSER_H
#define MATRIQUEUSER_H

#include "user.h"
#include "room.h"

#include <QObject>

using namespace QMatrixClient;

class MatriqueUser : public User {
  Q_OBJECT
  Q_PROPERTY(QImage avatar READ getAvatar NOTIFY inheritedAvatarChanged)
 public:
  MatriqueUser(QString userId, Connection* connection);

  QImage getAvatar() { return avatar(128); }

 signals:
  void inheritedAvatarChanged(User* user, const Room* roomContext); // https://bugreports.qt.io/browse/QTBUG-7684
};

#endif  // MATRIQUEUSER_H
