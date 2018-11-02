#include "spectraluser.h"

SpectralUser::SpectralUser(QString userId, Connection* connection)
    : User(userId, connection) {
  connect(this, &User::avatarChanged, this,
          &SpectralUser::inheritedAvatarChanged);
}
