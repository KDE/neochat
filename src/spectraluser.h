#ifndef SpectralUser_H
#define SpectralUser_H

#include <QObject>

#include "room.h"
#include "user.h"

using namespace Quotient;

class SpectralUser : public User
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color CONSTANT)
public:
    SpectralUser(QString userId, Connection *connection)
        : User(userId, connection)
    {
    }

    QColor color();
};

#endif // SpectralUser_H
