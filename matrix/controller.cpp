#include "controller.h"

#include "libqmatrixclient/connection.h"

Controller::Controller(QObject *parent) : QObject(parent) {

}

Controller::~Controller() {

}

void Controller::login(QString home, QString user, QString pass) {
    if(!isLogin) {
        if(home.isEmpty()) home = "matrix.org";

        qDebug() << "UserID:" << userID;
        qDebug() << "Token:" << token;
        qDebug() << "Home:" << home;
        qDebug() << "User:" << user;
        qDebug() << "Pass:" << pass;

        if(!userID.isEmpty() && !token.isEmpty()) {
            qDebug() << "Using token.";
            m_connection->connectWithToken(userID, token, "");
        } else if(!user.isEmpty() && !pass.isEmpty()) {
            qDebug() << "Using given credential.";
            m_connection->connectToServer("@"+user+":"+home, pass, "");
        }
    } else {
        qDebug() << "You are already logged in.";
    }
}

void Controller::setConnection(QMatrixClient::Connection* conn) {
    m_connection = conn;
    connect(m_connection, &QMatrixClient::Connection::connected, this, &Controller::connected);
    connect(m_connection, &QMatrixClient::Connection::resolveError, this, &Controller::reconnect);
    connect(m_connection, &QMatrixClient::Connection::syncError, this, &Controller::reconnect);
    connect(m_connection, &QMatrixClient::Connection::syncDone, this, &Controller::resync);
    emit connectionChanged();
}

void Controller::logout() {
    userID = "";
    token = "";
    setIsLogin(false);
}

void Controller::connected() {
    setUserID(m_connection->userId());
    setToken(m_connection->accessToken());
    m_connection->loadState();
    resync();
    setIsLogin(true);
}

void Controller::resync() {
    qDebug() << "Syncing Matrix.";
    m_connection->sync(30000);
    m_connection->saveState();
}

void Controller::reconnect() {
    qDebug() << "Connection lost. Reconnecting...";
    m_connection->connectWithToken(userID, token, "");
}
