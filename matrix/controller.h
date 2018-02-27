#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>

#include "libqmatrixclient/connection.h"

using namespace QMatrixClient;

class Controller : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString userID READ getUserID WRITE setUserID NOTIFY userIDChanged)
    Q_PROPERTY(QByteArray token READ getToken WRITE setToken NOTIFY tokenChanged)
public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller();

    // All the Q_INVOKABLEs.
    Q_INVOKABLE void init();
    Q_INVOKABLE void login(QString, QString, QString);
    Q_INVOKABLE void logout();

    // All the non-Q_INVOKABLE functions.

    // All the Q_PROPERTYs.
    QString userID;
    QString getUserID() { return userID; }
    void setUserID(QString n) {
        if(n != userID) {
            userID = n;
            emit userIDChanged();
        }
    }

    QByteArray token;
    QByteArray getToken() { return token; }
    void setToken(QByteArray n) {
        if(n != token) {
            token = n;
            emit tokenChanged();
        }
    }

private:
    QMatrixClient::Connection *connection = new QMatrixClient::Connection();
    void resync();
    void reconnect();

signals:
    void userIDChanged();
    void tokenChanged();
    void homeServerChanged();

public slots:
};

#endif // CONTROLLER_H
