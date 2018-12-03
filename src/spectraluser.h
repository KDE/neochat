#ifndef SpectralUser_H
#define SpectralUser_H

#include "room.h"
#include "user.h"

#include <QObject>

using namespace QMatrixClient;

class SpectralUser : public User {
  Q_OBJECT
 public:
  SpectralUser(QString userId, Connection* connection)
      : User(userId, connection) {}
};

#endif  // SpectralUser_H
