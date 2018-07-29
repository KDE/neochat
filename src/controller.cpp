#include "controller.h"

#include "connection.h"
#include "events/eventcontent.h"
#include "events/roommessageevent.h"

#include <QFile>
#include <QImage>
#include <QMimeDatabase>

Controller::Controller(QObject* parent) : QObject(parent) {
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

Controller::~Controller() {
  m_connection->saveState();
  m_connection->stopSync();
  m_connection->deleteLater();
}

void Controller::login() {
  if (!isLogin) {
    m_connection->setHomeserver(QUrl(homeserver));
    m_connection->connectWithToken(userID, token, "");
  }
}

void Controller::loginWithCredentials(QString serverAddr, QString user,
                                      QString pass) {
  if (!isLogin) {
    if (!user.isEmpty() && !pass.isEmpty()) {
      m_connection->setHomeserver(QUrl(serverAddr));
      m_connection->connectToServer(user, pass, "");
    }
  } else {
    qCritical() << "You are already logged in.";
  }
}

void Controller::logout() {
  setUserID("");
  setToken("");
  setIsLogin(false);
}

void Controller::connected() {
  setHomeserver(m_connection->homeserver().toString());
  setUserID(m_connection->userId());
  setToken(m_connection->accessToken());
  m_connection->loadState();
  resync();
  setIsLogin(true);
}

void Controller::resync() {
  m_connection->sync(30000);
  m_connection->saveState();
}

void Controller::reconnect() {
  qDebug() << "Connection lost. Reconnecting...";
  m_connection->connectWithToken(userID, token, "");
}

void Controller::postFile(QMatrixClient::Room* room, const QUrl& localFile,
                          const QUrl& mxcUrl) {
  const QString mime = getMIME(localFile);
  const QString fileName = localFile.toLocalFile();
  QString msgType = "m.file";
  if (mime.startsWith("image")) msgType = "m.image";
  if (mime.startsWith("video")) msgType = "m.video";
  if (mime.startsWith("audio")) msgType = "m.audio";
  QJsonObject json{QJsonObject{{"msgtype", msgType},
                               {"body", fileName},
                               {"filename", fileName},
                               {"url", mxcUrl.url()}}};
  room->postMessage("m.room.message", json);
}

QString Controller::getMIME(const QUrl& fileUrl) const {
  QMimeDatabase* db = new QMimeDatabase();
  const QString mime = db->mimeTypeForFile(fileUrl.toLocalFile()).name();
  delete db;
  return mime;
}
