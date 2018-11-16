#include "controller.h"

#include "settings.h"
#include "spectralroom.h"
#include "spectraluser.h"

#include "events/eventcontent.h"
#include "events/roommessageevent.h"

#include "csapi/joining.h"
#include "csapi/logout.h"

#include <QClipboard>
#include <QFile>
#include <QFileInfo>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QtCore/QStringBuilder>
#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopServices>
#include <QtGui/QMovie>
#include <QtGui/QPixmap>
#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QNetworkReply>

Controller::Controller(QObject* parent)
    : QObject(parent), notificationsManager(this) {
  QApplication::setQuitOnLastWindowClosed(false);

  connect(&notificationsManager, &NotificationsManager::notificationClicked,
          this, &Controller::notificationClicked);

  Connection::setRoomType<SpectralRoom>();
  Connection::setUserType<SpectralUser>();

  QTimer::singleShot(0, this, SLOT(invokeLogin()));
}

Controller::~Controller() {
  for (Connection* c : m_connections) {
    c->saveState();
    c->stopSync();
  }
}

inline QString accessTokenFileName(const AccountSettings& account) {
  QString fileName = account.userId();
  fileName.replace(':', '_');
  return QStandardPaths::writableLocation(
             QStandardPaths::AppLocalDataLocation) +
         '/' + fileName;
}

void Controller::loginWithCredentials(QString serverAddr, QString user,
                                      QString pass) {
  if (!user.isEmpty() && !pass.isEmpty()) {
    Connection* m_connection = new Connection(this);
    m_connection->setHomeserver(QUrl(serverAddr));
    m_connection->connectToServer(user, pass, "");
    connect(m_connection, &Connection::connected, [=] {
      AccountSettings account(m_connection->userId());
      account.setKeepLoggedIn(true);
      account.clearAccessToken();  // Drop the legacy - just in case
      account.setHomeserver(m_connection->homeserver());
      account.setDeviceId(m_connection->deviceId());
      account.setDeviceName("Spectral");
      if (!saveAccessToken(account, m_connection->accessToken()))
        qWarning() << "Couldn't save access token";
      account.sync();
      addConnection(m_connection);
    });
    connect(m_connection, &Connection::networkError,
            [=](QString error, QByteArray detail) {
              emit errorOccured("Network Error", error);
            });
    connect(m_connection, &Connection::loginError,
            [=](QString error, QByteArray detail) {
              emit errorOccured("Login Failed", error);
            });
  }
}

void Controller::logout(Connection* conn) {
  if (!conn) {
    qCritical() << "Attempt to logout null connection";
    return;
  }

  SettingsGroup("Accounts").remove(conn->userId());
  QFile(accessTokenFileName(AccountSettings(conn->userId()))).remove();

  auto job = conn->callApi<LogoutJob>();
  connect(job, &LogoutJob::finished, conn, [=] {
    conn->stopSync();
    emit conn->stateChanged();
    emit conn->loggedOut();
  });
  connect(job, &LogoutJob::failure, this, [=] {
    emit errorOccured("Server-side Logout Failed", job->errorString());
  });
}

void Controller::addConnection(Connection* c) {
  Q_ASSERT_X(c, __FUNCTION__, "Attempt to add a null connection");

  m_connections.push_back(c);

  connect(c, &Connection::syncDone, this, [=] {
    emit syncDone();
    c->sync(30000);

    static int counter = 0;
    if (++counter % 17 == 2) c->saveState();
  });
  connect(c, &Connection::loggedOut, this, [=] { dropConnection(c); });

  using namespace QMatrixClient;

  c->sync(30000);

  emit connectionAdded(c);
}

void Controller::dropConnection(Connection* c) {
  Q_ASSERT_X(c, __FUNCTION__, "Attempt to drop a null connection");
  m_connections.removeOne(c);

  emit connectionDropped(c);
  c->deleteLater();
}

void Controller::invokeLogin() {
  using namespace QMatrixClient;
  const auto accounts = SettingsGroup("Accounts").childGroups();
  for (const auto& accountId : accounts) {
    AccountSettings account{accountId};
    if (!account.homeserver().isEmpty()) {
      auto accessToken = loadAccessToken(account);

      auto c = new Connection(account.homeserver(), this);
      auto deviceName = account.deviceName();
      connect(c, &Connection::connected, this, [=] {
        c->loadState();
        addConnection(c);
      });
      connect(c, &Connection::loginError,
              [=](QString error, QByteArray detail) {
                emit errorOccured("Login Failed", error);
              });
      connect(c, &Connection::networkError,
              [=](QString error, QByteArray detail) {
                emit errorOccured("Network Error", error);
              });
      c->connectWithToken(account.userId(), accessToken, account.deviceId());
    }
  }
  if (!m_connections.isEmpty()) setConnection(m_connections[0]);
  emit initiated();
}

QByteArray Controller::loadAccessToken(const AccountSettings& account) {
  QFile accountTokenFile{accessTokenFileName(account)};
  if (accountTokenFile.open(QFile::ReadOnly)) {
    if (accountTokenFile.size() < 1024) return accountTokenFile.readAll();

    qWarning() << "File" << accountTokenFile.fileName() << "is"
               << accountTokenFile.size()
               << "bytes long - too long for a token, ignoring it.";
  }
  qWarning() << "Could not open access token file"
             << accountTokenFile.fileName();

  return {};
}

bool Controller::saveAccessToken(const AccountSettings& account,
                                 const QByteArray& accessToken) {
  // (Re-)Make a dedicated file for access_token.
  QFile accountTokenFile{accessTokenFileName(account)};
  accountTokenFile.remove();  // Just in case

  auto fileDir = QFileInfo(accountTokenFile).dir();
  if (!((fileDir.exists() || fileDir.mkpath(".")) &&
        accountTokenFile.open(QFile::WriteOnly))) {
    emit errorOccured("I/O Denied", "Cannot save access token.");
  } else {
    accountTokenFile.write(accessToken);
    return true;
  }
  return false;
}

void Controller::joinRoom(Connection* c, const QString& alias) {
  JoinRoomJob* joinRoomJob = c->joinRoom(alias);
  joinRoomJob->connect(joinRoomJob, &JoinRoomJob::failure, [=] {
    emit errorOccured("Join Room Failed", joinRoomJob->errorString());
  });
}

void Controller::createRoom(Connection* c, const QString& name,
                            const QString& topic) {
  CreateRoomJob* createRoomJob =
      c->createRoom(Connection::PublishRoom, "", name, topic, QStringList());
  createRoomJob->connect(createRoomJob, &CreateRoomJob::failure, [=] {
    emit errorOccured("Create Room Failed", createRoomJob->errorString());
  });
}

void Controller::createDirectChat(Connection* c, const QString& userID) {
  CreateRoomJob* createRoomJob = c->createDirectChat(userID);
  createRoomJob->connect(createRoomJob, &CreateRoomJob::failure, [=] {
    emit errorOccured("Create Direct Chat Failed",
                      createRoomJob->errorString());
  });
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

void Controller::postNotification(const QString& roomId, const QString& eventId,
                                  const QString& roomName,
                                  const QString& senderName,
                                  const QString& text, const QImage& icon,
                                  const QUrl& iconPath) {
  notificationsManager.postNotification(roomId, eventId, roomName, senderName,
                                        text, icon, iconPath);
}
