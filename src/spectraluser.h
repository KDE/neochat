#ifndef SpectralUser_H
#define SpectralUser_H

#include "paintable.h"
#include "room.h"
#include "user.h"

#include <QObject>
#include <QPointer>

using namespace QMatrixClient;

class UserPaintable : public Paintable {
  Q_OBJECT
 public:
  UserPaintable(User* parent) : Paintable(parent), m_user(parent) {}

  QImage image(int dimension) override {
    if (!m_user) return {};
    return m_user->avatar(dimension);
  }
  QImage image(int width, int height) override {
    if (!m_user) return {};
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
      : User(userId, connection), m_paintable(this) {}

  Paintable* paintable() { return &m_paintable; }

 private:
  UserPaintable m_paintable;
};

#endif  // SpectralUser_H
