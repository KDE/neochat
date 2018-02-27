#include "controller.h"

#include "libqmatrixclient/connection.h"

Controller::Controller(QObject *parent) : QObject(parent) {
    connect(connection, &Connection::connected, this, &Controller::connected);
    connect(connection, &Connection::resolveError, this, &Controller::reconnect);
    connect(connection, &Connection::syncError, this, &Controller::reconnect);
    connect(connection, &Connection::syncDone, this, &Controller::resync);
}

Controller::~Controller() {

}

void Controller::login(QString home, QString user, QString pass) {
    if(home.isEmpty()) home = "matrix.org";

    qDebug() << "UserID:" << userID;
    qDebug() << "Token:" << token;
    qDebug() << "Home:" << home;
    qDebug() << "User:" << user;
    qDebug() << "Pass:" << pass;

    if(!userID.isEmpty() && !token.isEmpty()) {
        qDebug() << "Using token.";
        connection->connectWithToken(userID, token, "");
    } else if(!user.isEmpty() && !pass.isEmpty()) {
        qDebug() << "Using given credential.";
        connection->connectToServer("@"+user+":"+home, pass, "");
    }
}

void Controller::logout() {
    userID = "";
    token = "";
    setIsLogin(false);
}

void Controller::connected() {
    setUserID(connection->userId());
    setToken(connection->accessToken());
    setIsLogin(true);
}

void Controller::resync() {
    connection->sync(30000);
}

void Controller::reconnect() {
    Controller::connection->connectWithToken(userID, token, "");
}
