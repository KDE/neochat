#include "controller.h"

#include "connection.h"

Controller::Controller(QObject *parent) : QObject(parent) {
  connect(m_connection, &QMatrixClient::Connection::connected, this,
          &Controller::connected);
  connect(m_connection, &QMatrixClient::Connection::resolveError, this,
          &Controller::reconnect);
  connect(m_connection, &QMatrixClient::Connection::syncError, this,
          &Controller::reconnect);
  connect(m_connection, &QMatrixClient::Connection::syncDone, this,
          &Controller::resync);
  connect(m_connection, &QMatrixClient::Connection::connected, this,
          &Controller::connectionChanged);

  connect(m_connection, &QMatrixClient::Connection::connected,
          [=] { setBusy(true); });
  connect(m_connection, &QMatrixClient::Connection::syncDone,
          [=] { setBusy(false); });
}

Controller::~Controller() { m_connection->stopSync(); }

void Controller::login() {
  if (!isLogin) {
    qDebug() << "UserID:" << userID;
    qDebug() << "Token:" << token;

    m_connection->setHomeserver(QUrl(homeserver));
    m_connection->connectWithToken(userID, token, "");
  }
}

void Controller::loginWithCredentials(QString serverAddr, QString user,
                                      QString pass) {
  if (!isLogin) {
    qDebug() << "Server:" << serverAddr;
    qDebug() << "User:" << user;
    qDebug() << "Pass:" << pass;

    if (!user.isEmpty() && !pass.isEmpty()) {
      qDebug() << "Using given credential.";
      m_connection->setHomeserver(QUrl(serverAddr));
      m_connection->connectToServer(user, pass, "");
    }
  } else {
    qDebug() << "You are already logged in.";
  }
}

void Controller::logout() {
  qDebug() << "Logging out.";
  setUserID("");
  setToken("");
  setIsLogin(false);
}

void Controller::uploadFile(QString filename) {
  m_connection->uploadFile(filename);
}

void Controller::connected() {
  qDebug() << "Logged in.";
  setHomeserver(m_connection->homeserver().toString());
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
