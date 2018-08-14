#include "controller.h"

#include "connection.h"
#include "events/eventcontent.h"
#include "events/roommessageevent.h"

#include "csapi/create_room.h"
#include "csapi/joining.h"
#include "csapi/leaving.h"

#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QImage>

Controller::Controller(QObject* parent) : QObject(parent) {
  connect(m_connection, &Connection::connected, this, &Controller::connected);
  connect(m_connection, &Connection::resolveError, this,
          &Controller::reconnect);
  connect(m_connection, &Connection::syncError, this, &Controller::reconnect);
  connect(m_connection, &Connection::syncDone, this, &Controller::resync);
  connect(m_connection, &Connection::connected, this,
          &Controller::connectionChanged);

  connect(m_connection, &Connection::connected, [=] { setBusy(true); });
  connect(m_connection, &Connection::syncDone, [=] { setBusy(false); });
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
  m_connection->logout();
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

void Controller::resync() { m_connection->sync(30000); }

void Controller::reconnect() {
  qDebug() << "Connection lost. Reconnecting...";
  m_connection->connectWithToken(userID, token, "");
}

void Controller::uploadFile(Room* room) {
  if (!room) return;
  auto localFile = QFileDialog::getOpenFileUrl(Q_NULLPTR, tr("Save File as"));
  if (!localFile.isEmpty()) {
    room->uploadFile(localFile.toString(), localFile, getMIME(localFile));
    QMetaObject::Connection* const connection = new QMetaObject::Connection;
    *connection = connect(room, &Room::fileTransferCompleted,
                          [=](QString id, QUrl localFile, QUrl mxcUrl) {
                            disconnect(*connection);
                            postFile(room, localFile, mxcUrl);
                          });
  }
}

void Controller::postFile(Room* room, const QUrl& localFile,
                          const QUrl& mxcUrl) {
  const QString mime = getMIME(localFile);
  const QString fileName = localFile.fileName();
  QString msgType = "m.file";
  if (mime.startsWith("image")) msgType = "m.image";
  if (mime.startsWith("video")) msgType = "m.video";
  if (mime.startsWith("audio")) msgType = "m.audio";
  QJsonObject json{QJsonObject{{"msgtype", msgType},
                               {"body", fileName},
                               {"filename", fileName},
                               {"url", mxcUrl.url()}}};
  room->postJson("m.room.message", json);
}

QString Controller::getMIME(const QUrl& fileUrl) const {
  const QString mime = m_db.mimeTypeForFile(fileUrl.toLocalFile()).name();
  return mime;
}

void Controller::forgetRoom(const QString& roomID) {
  ForgetRoomJob* forgetRoomJob = m_connection->forgetRoom(roomID);
  setBusy(true);
  forgetRoomJob->connect(forgetRoomJob, &ForgetRoomJob::finished,
                         [=] { setBusy(false); });
}

void Controller::joinRoom(const QString& alias) {
  JoinRoomJob* joinRoomJob = m_connection->joinRoom(alias);
  setBusy(true);
  joinRoomJob->connect(joinRoomJob, &JoinRoomJob::finished,
                       [=] { setBusy(false); });
}

void Controller::createRoom(const QString& name, const QString& topic) {
  CreateRoomJob* createRoomJob = m_connection->createRoom(
      Connection::PublishRoom, "", name, topic, QStringList());
  setBusy(true);
  createRoomJob->connect(createRoomJob, &CreateRoomJob::finished,
                         [=] { setBusy(false); });
}

void Controller::createDirectChat(const QString& userID) {
  m_connection->requestDirectChat(userID);
}

void Controller::copyToClipboard(const QString& text) {
  m_clipboard->setText(text);
}

void Controller::saveFileAs(Room* room, QString eventId) {
  if (!room) return;
  auto fileName = QFileDialog::getSaveFileName(
      Q_NULLPTR, tr("Save File as"), room->fileNameToDownload(eventId));
  if (!fileName.isEmpty())
    room->downloadFile(eventId, QUrl::fromLocalFile(fileName));
}
