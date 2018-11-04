#ifndef SpectralUser_H
#define SpectralUser_H

#include "paintable.h"
#include "room.h"
#include "user.h"

#include <QObject>

using namespace QMatrixClient;

class UserPaintable : public Paintable {
  Q_OBJECT
 public:
  UserPaintable(User* parent) : Paintable(parent), m_user(parent) {
    connect(m_user, &User::avatarChanged, [=] { emit paintableChanged(); });
  }

  QImage image(int dimension) override {
    if (!m_user) return QImage();
    return m_user->avatar(dimension);
  }
  QImage image(int width, int height) override {
    if (!m_user) return QImage();
    return m_user->avatar(width, height);
  }

 private:
  User* m_user;
};

class SpectralUser : public User {
  Q_OBJECT
  Q_PROPERTY(Paintable* paintable READ paintable CONSTANT)
 public:
  SpectralUser(QString userId, Connection* connection)
      : User(userId, connection) {}

  Paintable* paintable() { return new UserPaintable(this); }
};

#endif  // SpectralUser_H
