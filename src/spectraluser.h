#ifndef SpectralUser_H
#define SpectralUser_H

#include "room.h"
#include "user.h"

#include <QObject>

using namespace QMatrixClient;

class SpectralUser : public User {
  Q_OBJECT
  Q_PROPERTY(QColor color READ color CONSTANT)
 public:
  SpectralUser(QString userId, Connection* connection)
      : User(userId, connection) {}

  QColor color();
};

#endif  // SpectralUser_H
