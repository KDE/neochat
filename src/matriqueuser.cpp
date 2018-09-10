#include "matriqueuser.h"

MatriqueUser::MatriqueUser(QString userId, Connection* connection)
    : User(userId, connection) {
    connect(this, &User::avatarChanged, this, &MatriqueUser::inheritedAvatarChanged);
}
