#include "controller.h"

#include "libqmatrixclient/connection.h"

Controller::Controller(QObject *parent) : QObject(parent) {
    connect(m_connection, &QMatrixClient::Connection::connected, this, &Controller::connected);
    connect(m_connection, &QMatrixClient::Connection::resolveError, this, &Controller::reconnect);
    connect(m_connection, &QMatrixClient::Connection::syncError, this, &Controller::reconnect);
    connect(m_connection, &QMatrixClient::Connection::syncDone, this, &Controller::resync);
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
        m_connection->connectWithToken(userID, token, "");
    } else if(!user.isEmpty() && !pass.isEmpty()) {
        qDebug() << "Using given credential.";
        m_connection->connectToServer("@"+user+":"+home, pass, "");
    }
}

void Controller::logout() {
    userID = "";
    token = "";
    setIsLogin(false);
}

void Controller::connected() {
    setUserID(m_connection->userId());
    setToken(m_connection->accessToken());
    roomListModel->init(m_connection);
    resync();
    setIsLogin(true);
}

void Controller::resync() {
    m_connection->sync(30000);
}

void Controller::reconnect() {
    m_connection->connectWithToken(userID, token, "");
}
