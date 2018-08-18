#include "controller.h"

#include "matriqueroom.h"

#include "events/eventcontent.h"
#include "events/roommessageevent.h"

#include "csapi/joining.h"

#include <QClipboard>

Controller::Controller(QObject* parent) : QObject(parent) {
  Connection::setRoomType<MatriqueRoom>();

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
  if (!m_isLogin) {
    m_connection->setHomeserver(QUrl(m_homeserver));
    m_connection->connectWithToken(m_userID, m_token, "");
  }
}

void Controller::loginWithCredentials(QString serverAddr, QString user,
                                      QString pass) {
  if (!m_isLogin) {
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
  m_connection->connectWithToken(m_userID, m_token, "");
}

void Controller::joinRoom(const QString& alias) {
  JoinRoomJob* joinRoomJob = m_connection->joinRoom(alias);
  setBusy(true);
  joinRoomJob->connect(joinRoomJob, &JoinRoomJob::finished,
                       [=] { setBusy(false); });
}

void Controller::createRoom(const QString& name, const QString& topic) {
  CreateRoomJob* createRoomJob =
      ((Connection*)m_connection)
          ->createRoom(Connection::PublishRoom, "", name, topic, QStringList());
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

void Controller::playAudio(QUrl localFile) {
  QMediaPlayer* player = new QMediaPlayer;
  player->setMedia(localFile);
  player->play();
  connect(player, &QMediaPlayer::stateChanged, [=] { player->deleteLater(); });
}
