#include "controller.h"

#include "libqmatrixclient/connection.h"

Controller::Controller(QObject *parent) : QObject(parent) {

}

Controller::~Controller() {

}

void Controller::init() {
    connect(connection, &Connection::connected,
        [=](){
            qInfo() << "Matrix connected.";
            setUserID(connection->userId());
            setToken(connection->accessToken());
        }
    );

    connect(connection, &Connection::resolveError, this, &Controller::reconnect);
    connect(connection, &Connection::syncError, this, &Controller::reconnect);
    connect(connection, &Connection::syncDone, this, &Controller::resync);
}

void Controller::login(QString home, QString user, QString pass) {
    qInfo() << "UserID:" << userID;
    qInfo() << "Token:" << token;
    qInfo() << "Home:" << home;
    qInfo() << "User:" << user;
    qInfo() << "Pass:" << pass;
    if(!userID.isEmpty() && !token.isEmpty()) {
        qInfo() << "Using token.";
        connection->connectWithToken(userID, token, "");

    } else if(!user.isEmpty() && !pass.isEmpty()) {
        qInfo() << "Using given credential.";
        connection->connectToServer("@"+user+":"+home, pass, "");
    }
}

void Controller::logout() {
    userID = "";
    token = "";
}

void Controller::resync() {
    connection->sync(30000);
}

void Controller::reconnect() {
    Controller::connection->connectWithToken(userID, token, "");
}
