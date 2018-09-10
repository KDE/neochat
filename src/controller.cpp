#include "controller.h"

#include "matriqueroom.h"
#include "settings.h"

#include "events/eventcontent.h"
#include "events/roommessageevent.h"

#include "csapi/joining.h"

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

Controller::Controller(QObject* parent) : QObject(parent) {
  tray->setIcon(QIcon(":/asset/img/icon.png"));
  tray->setToolTip("Matrique");
  connect(tray, &QSystemTrayIcon::activated,
          [this](QSystemTrayIcon::ActivationReason r) {
            if (r != QSystemTrayIcon::Context) emit toggleWindow();
          });
  connect(tray, &QSystemTrayIcon::messageClicked, [=] { emit toggleWindow(); });
  trayMenu->addAction("Toggle Window", [=] { emit toggleWindow(); });
  trayMenu->addAction("Quit", [=] { QApplication::quit(); });
  tray->setContextMenu(trayMenu);
  tray->show();

  Connection::setRoomType<MatriqueRoom>();

  QTimer::singleShot(0, this, SLOT(invokeLogin()));
}

Controller::~Controller() {}

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
      account.setDeviceName("Matrique");
      if (!saveAccessToken(account, m_connection->accessToken()))
        qWarning() << "Couldn't save access token";
      account.sync();
      addConnection(m_connection);
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

  conn->logout();
}

void Controller::addConnection(Connection* c) {
  Q_ASSERT_X(c, __FUNCTION__, "Attempt to add a null connection");

  m_connections.push_back(c);

  connect(c, &Connection::syncDone, this, [=] {
    c->saveState();
    c->sync(30000);
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
      if (accessToken.isEmpty()) {
        // Try to look in the legacy location (QSettings) and if found,
        // migrate it from there to a file.
        accessToken = account.accessToken().toLatin1();
        if (accessToken.isEmpty())
          continue;  // No access token anywhere, no autologin

        saveAccessToken(account, accessToken);
        account.clearAccessToken();  // Clean the old place
      }

      auto c = new Connection(account.homeserver(), this);
      auto deviceName = account.deviceName();
      connect(c, &Connection::connected, this, [=] {
        c->loadState();
        addConnection(c);
      });
      c->connectWithToken(account.userId(), accessToken, account.deviceId());
    }
  }
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
    emit errorOccured();
  } else {
    // Try to restrict access rights to the file. The below is useless
    // on Windows: FAT doesn't control access at all and NTFS is
    // incompatible with the UNIX perms model used by Qt. If the attempt
    // didn't have the effect, at least ask the user if it's fine to save
    // the token to a file readable by others.
    // TODO: use system-specific API to ensure proper access.
    if ((accountTokenFile.setPermissions(QFile::ReadOwner |
                                         QFile::WriteOwner) &&
         !(accountTokenFile.permissions() &
           (QFile::ReadGroup | QFile::ReadOther)))) {
      accountTokenFile.write(accessToken);
      return true;
    }
  }
  return false;
}

void Controller::joinRoom(Connection* c, const QString& alias) {
  JoinRoomJob* joinRoomJob = c->joinRoom(alias);
  setBusy(true);
  joinRoomJob->connect(joinRoomJob, &JoinRoomJob::finished,
                       [=] { setBusy(false); });
}

void Controller::createRoom(Connection* c, const QString& name,
                            const QString& topic) {
  CreateRoomJob* createRoomJob =
      c->createRoom(Connection::PublishRoom, "", name, topic, QStringList());
  setBusy(true);
  createRoomJob->connect(createRoomJob, &CreateRoomJob::finished,
                         [=] { setBusy(false); });
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

void Controller::showMessage(const QString& title, const QString& msg,
                             const QIcon& icon) {
  tray->showMessage(title, msg, icon);
}
