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
    Q_PROPERTY(QString defaultName READ defaultName WRITE setDefaultName NOTIFY nameChanged)
public:
    SpectralUser(QString userId, Connection *connection)
        : User(userId, connection)
    {
    }

    QColor color();

    //TODO libQuotient 0.7: remove
    void setDefaultName(QString defaultName);
    QString defaultName();

Q_SIGNALS:
    void nameChanged();

private:
    QString m_defaultName;
};

#endif // SpectralUser_H
