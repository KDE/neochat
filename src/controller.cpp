/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "controller.h"

#ifndef Q_OS_ANDROID
#include <qt5keychain/keychain.h>
#else
#include <KConfig>
#include <KConfigGroup>
#endif

#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QSysInfo>
#include <QTimer>
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopServices>
#include <QtGui/QMovie>
#include <QtGui/QPixmap>
#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QNetworkReply>

#include "csapi/account-data.h"
#include "csapi/content-repo.h"
#include "csapi/joining.h"
#include "csapi/logout.h"
#include "csapi/profile.h"
#include "csapi/registration.h"
#include "events/eventcontent.h"
#include "events/roommessageevent.h"
#include "neochatroom.h"
#include "neochatuser.h"
#include "settings.h"
#include "utils.h"

Controller::Controller(QObject *parent)
    : QObject(parent)
{
    QApplication::setQuitOnLastWindowClosed(false);

    Connection::setRoomType<NeoChatRoom>();
    Connection::setUserType<NeoChatUser>();

    connect(&m_ncm, &QNetworkConfigurationManager::onlineStateChanged, this, &Controller::isOnlineChanged);

    QTimer::singleShot(0, this, [=] {
        invokeLogin();
    });
}

Controller::~Controller()
{
    for (auto c : qAsConst(m_connections)) {
        c->stopSync();
        c->saveState();
    }
}

Controller &Controller::instance()
{
    static Controller _instance;
    return _instance;
}

inline QString accessTokenFileName(const AccountSettings &account)
{
    QString fileName = account.userId();
    fileName.replace(':', '_');
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + '/' + fileName;
}

void Controller::loginWithCredentials(QString serverAddr, QString user, QString pass, QString deviceName)
{
    if (user.isEmpty() || pass.isEmpty()) {
        return;
    }

    if (deviceName.isEmpty()) {
        deviceName = "NeoChat " + QSysInfo::machineHostName() + " " + QSysInfo::productType() + " " + QSysInfo::productVersion() + " " + QSysInfo::currentCpuArchitecture();
    }

    QUrl serverUrl(serverAddr);

    auto conn = new Connection(this);
    if (serverUrl.isValid()) {
        conn->setHomeserver(serverUrl);
    }
    conn->connectToServer(user, pass, deviceName, "");

    connect(conn, &Connection::connected, this, [=] {
        AccountSettings account(conn->userId());
        account.setKeepLoggedIn(true);
        account.clearAccessToken(); // Drop the legacy - just in case
        account.setHomeserver(conn->homeserver());
        account.setDeviceId(conn->deviceId());
        account.setDeviceName(deviceName);
        if (!saveAccessTokenToKeyChain(account, conn->accessToken()))
            qWarning() << "Couldn't save access token";
        account.sync();
        addConnection(conn);
        setConnection(conn);
    });
    connect(conn, &Connection::networkError, [=](QString error, QString, int, int) {
        Q_EMIT errorOccured("Network Error", error);
    });
    connect(conn, &Connection::loginError, [=](QString error, QString) {
        Q_EMIT errorOccured("Login Failed", error);
    });
}

void Controller::loginWithAccessToken(QString serverAddr, QString user, QString token, QString deviceName)
{
    if (user.isEmpty() || token.isEmpty()) {
        return;
    }

    QUrl serverUrl(serverAddr);

    auto conn = new Connection(this);
    if (serverUrl.isValid()) {
        conn->setHomeserver(serverUrl);
    }

    connect(conn, &Connection::connected, this, [=] {
        AccountSettings account(conn->userId());
        account.setKeepLoggedIn(true);
        account.clearAccessToken(); // Drop the legacy - just in case
        account.setHomeserver(conn->homeserver());
        account.setDeviceId(conn->deviceId());
        account.setDeviceName(deviceName);
        if (!saveAccessTokenToKeyChain(account, conn->accessToken()))
            qWarning() << "Couldn't save access token";
        account.sync();
        addConnection(conn);
        setConnection(conn);
    });
    connect(conn, &Connection::networkError, this, [=](QString error, QString, int, int) {
        Q_EMIT errorOccured("Network Error", error);
    });
    conn->connectWithToken(user, token, deviceName);
}

void Controller::logout(Connection *conn)
{
    if (!conn) {
        qCritical() << "Attempt to logout null connection";
        return;
    }

    SettingsGroup("Accounts").remove(conn->userId());
    QFile(accessTokenFileName(AccountSettings(conn->userId()))).remove();

#ifndef Q_OS_ANDROID
    QKeychain::DeletePasswordJob job(qAppName());
    job.setAutoDelete(true);
    job.setKey(conn->userId());
    QEventLoop loop;
    QKeychain::DeletePasswordJob::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
#else
    KConfig config("neochat_tokens");
    KConfigGroup tokensGroup(&config, "Tokens");
    tokensGroup.deleteEntry(conn->userId());
#endif

    auto logoutJob = conn->callApi<LogoutJob>();
    connect(logoutJob, &LogoutJob::finished, conn, [=] {
        conn->stopSync();
        Q_EMIT conn->stateChanged();
        Q_EMIT conn->loggedOut();
        if (!m_connections.isEmpty())
            setConnection(m_connections[0]);
        else
            setConnection(nullptr);
    });
    connect(logoutJob, &LogoutJob::failure, this, [=] {
        Q_EMIT errorOccured("Server-side Logout Failed", logoutJob->errorString());
    });
}

void Controller::addConnection(Connection *c)
{
    Q_ASSERT_X(c, __FUNCTION__, "Attempt to add a null connection");

    m_connections += c;

    c->setLazyLoading(true);

    connect(c, &Connection::syncDone, this, [=] {
        setBusy(false);

        Q_EMIT syncDone();

        c->sync(30000);
        c->saveState();
    });
    connect(c, &Connection::loggedOut, this, [=] {
        dropConnection(c);
    });
    connect(&m_ncm, &QNetworkConfigurationManager::onlineStateChanged, [=](bool status) {
        if (!status) {
            return;
        }

        c->stopSync();
        c->sync(30000);
    });

    setBusy(true);

    c->sync();

    Q_EMIT connectionAdded(c);
}

void Controller::dropConnection(Connection *c)
{
    Q_ASSERT_X(c, __FUNCTION__, "Attempt to drop a null connection");
    m_connections.removeOne(c);

    Q_EMIT connectionDropped(c);
    c->deleteLater();
}

void Controller::invokeLogin()
{
    const auto accounts = SettingsGroup("Accounts").childGroups();
    for (const auto &accountId : accounts) {
        AccountSettings account {accountId};
        if (!account.homeserver().isEmpty()) {
            auto accessToken = loadAccessTokenFromKeyChain(account);

            auto c = new Connection(account.homeserver(), this);
            connect(c, &Connection::connected, this, [=] {
                c->loadState();
                addConnection(c);
            });
            connect(c, &Connection::loginError, this, [=](QString error, QString) {
                Q_EMIT errorOccured("Login Failed", error);
                logout(c);
            });
            connect(c, &Connection::networkError, this, [=](QString error, QString, int, int) {
                Q_EMIT errorOccured("Network Error", error);
            });
            c->connectWithToken(account.userId(), accessToken, account.deviceId());
        }
    }

    if (!m_connections.isEmpty()) {
        setConnection(m_connections[0]);
    }

    Q_EMIT initiated();
}

QByteArray Controller::loadAccessTokenFromFile(const AccountSettings &account)
{
    QFile accountTokenFile {accessTokenFileName(account)};
    if (accountTokenFile.open(QFile::ReadOnly)) {
        if (accountTokenFile.size() < 1024)
            return accountTokenFile.readAll();

        qWarning() << "File" << accountTokenFile.fileName() << "is" << accountTokenFile.size() << "bytes long - too long for a token, ignoring it.";
    }
    qWarning() << "Could not open access token file" << accountTokenFile.fileName();

    return {};
}

QByteArray Controller::loadAccessTokenFromKeyChain(const AccountSettings &account)
{
#ifndef Q_OS_ANDROID
    qDebug() << "Read the access token from the keychain for " << account.userId();
    QKeychain::ReadPasswordJob job(qAppName());
    job.setAutoDelete(false);
    job.setKey(account.userId());
    QEventLoop loop;
    QKeychain::ReadPasswordJob::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error() == QKeychain::Error::NoError) {
        return job.binaryData();
    }

    qWarning() << "Could not read the access token from the keychain: " << qPrintable(job.errorString());
    // no access token from the keychain, try token file
    auto accessToken = loadAccessTokenFromFile(account);
    if (job.error() == QKeychain::Error::EntryNotFound) {
        if (!accessToken.isEmpty()) {
            qDebug() << "Migrating the access token from file to the keychain for " << account.userId();
            bool removed = false;
            bool saved = saveAccessTokenToKeyChain(account, accessToken);
            if (saved) {
                QFile accountTokenFile {accessTokenFileName(account)};
                removed = accountTokenFile.remove();
            }
            if (!(saved && removed)) {
                qDebug() << "Migrating the access token from the file to the keychain "
                            "failed";
            }
        }
    }

    return accessToken;
#else
    qDebug() << "Saving access token to KConfig";
    KConfig config("neochat_tokens");
    KConfigGroup tokensGroup(&config, "Tokens");
    QString token = tokensGroup.readEntry(account.userId(), QString());
    return token.toLatin1();
#endif
}

bool Controller::saveAccessTokenToFile(const AccountSettings &account, const QByteArray &accessToken)
{
    // (Re-)Make a dedicated file for access_token.
    QFile accountTokenFile {accessTokenFileName(account)};
    accountTokenFile.remove(); // Just in case

    auto fileDir = QFileInfo(accountTokenFile).dir();
    if (!((fileDir.exists() || fileDir.mkpath(".")) && accountTokenFile.open(QFile::WriteOnly))) {
        Q_EMIT errorOccured("I/O Denied", "Cannot save access token.");
    } else {
        accountTokenFile.write(accessToken);
        return true;
    }
    return false;
}

bool Controller::saveAccessTokenToKeyChain(const AccountSettings &account, const QByteArray &accessToken)
{
#ifndef Q_OS_ANDROID
    qDebug() << "Save the access token to the keychain for " << account.userId();
    QKeychain::WritePasswordJob job(qAppName());
    job.setAutoDelete(false);
    job.setKey(account.userId());
    job.setBinaryData(accessToken);
    QEventLoop loop;
    QKeychain::WritePasswordJob::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error()) {
        qWarning() << "Could not save access token to the keychain: " << qPrintable(job.errorString());
        return saveAccessTokenToFile(account, accessToken);
    }

#else
    KConfig config("neochat_tokens");
    KConfigGroup tokensGroup(&config, "Tokens");
    tokensGroup.writeEntry(account.userId(), accessToken);
#endif
    return true;
}

void Controller::joinRoom(Connection *c, const QString &alias)
{
    if (!alias.contains(":"))
        return;

    auto knownServer = alias.mid(alias.indexOf(":") + 1);
    auto joinRoomJob = c->joinRoom(alias, QStringList {knownServer});
    joinRoomJob->connect(joinRoomJob, &JoinRoomJob::failure, [=] {
        Q_EMIT errorOccured("Join Room Failed", joinRoomJob->errorString());
    });
}

void Controller::createRoom(Connection *c, const QString &name, const QString &topic)
{
    auto createRoomJob = c->createRoom(Connection::PublishRoom, "", name, topic, QStringList());
    createRoomJob->connect(createRoomJob, &CreateRoomJob::failure, [=] {
        Q_EMIT errorOccured("Create Room Failed", createRoomJob->errorString());
    });
}

void Controller::createDirectChat(Connection *c, const QString &userID)
{
    auto createRoomJob = c->createDirectChat(userID);
    createRoomJob->connect(createRoomJob, &CreateRoomJob::failure, [=] {
        Q_EMIT errorOccured("Create Direct Chat Failed", createRoomJob->errorString());
    });
}

void Controller::playAudio(QUrl localFile)
{
    auto player = new QMediaPlayer;
    player->setMedia(localFile);
    player->play();
    connect(player, &QMediaPlayer::stateChanged, [=] {
        player->deleteLater();
    });
}

void Controller::changeAvatar(Connection *conn, QUrl localFile)
{
    auto job = conn->uploadFile(localFile.toLocalFile());
    if (isJobRunning(job)) {
        connect(job, &BaseJob::success, this, [conn, job] {
            conn->callApi<SetAvatarUrlJob>(conn->userId(), job->contentUri());
        });
    }
}

void Controller::markAllMessagesAsRead(Connection *conn)
{
    const auto rooms = conn->allRooms();
    for (auto room : rooms) {
        room->markAllMessagesAsRead();
    }
}

void Controller::setAboutData(KAboutData aboutData)
{
    m_aboutData = aboutData;
    Q_EMIT aboutDataChanged();
}

KAboutData Controller::aboutData() const
{
    return m_aboutData;
}

void Controller::changePassword(Connection *connection, const QString &currentPassword, const QString &newPassword)
{
    NeochatChangePasswordJob *job = connection->callApi<NeochatChangePasswordJob>(newPassword, false);
    connect(job, &BaseJob::result, this, [this, job, currentPassword, newPassword, connection] {
        if (job->error() == 103) {
            QJsonObject replyData = job->jsonData();
            QJsonObject authData;
            authData["session"] = replyData["session"];
            authData["password"] = currentPassword;
            authData["type"] = "m.login.password";
            authData["user"] = connection->user()->id();
            QJsonObject identifier = {{"type", "m.id.user"}, {"user", connection->user()->id()}};
            authData["identifier"] = identifier;
            NeochatChangePasswordJob *innerJob = connection->callApi<NeochatChangePasswordJob>(newPassword, false, authData);
            connect(innerJob, &BaseJob::success, this, [this]() {
                Q_EMIT passwordStatus(PasswordStatus::Success);
            });
            connect(innerJob, &BaseJob::failure, this, [innerJob, this]() {
                if (innerJob->jsonData()["errcode"] == "M_FORBIDDEN") {
                    Q_EMIT passwordStatus(PasswordStatus::Wrong);
                } else {
                    Q_EMIT passwordStatus(PasswordStatus::Other);
                }
            });
        }
    });
}

NeochatChangePasswordJob::NeochatChangePasswordJob(const QString &newPassword, bool logoutDevices, const Omittable<QJsonObject> &auth)
    : BaseJob(HttpVerb::Post, QStringLiteral("ChangePasswordJob"), QStringLiteral("/_matrix/client/r0") % "/account/password")
{
    QJsonObject _data;
    addParam<>(_data, QStringLiteral("new_password"), newPassword);
    addParam<IfNotEmpty>(_data, QStringLiteral("logout_devices"), logoutDevices);
    addParam<IfNotEmpty>(_data, QStringLiteral("auth"), auth);
    setRequestData(std::move(_data));
}

QVector<Connection *> Controller::connections() const
{
    return m_connections;
}

int Controller::accountCount() const
{
    return m_connections.count();
}

bool Controller::quitOnLastWindowClosed() const
{
    return QApplication::quitOnLastWindowClosed();
}

void Controller::setQuitOnLastWindowClosed(bool value)
{
    if (quitOnLastWindowClosed() != value) {
        QApplication::setQuitOnLastWindowClosed(value);
        Q_EMIT quitOnLastWindowClosedChanged();
    }
}

bool Controller::isOnline() const
{
    return m_ncm.isOnline();
}

bool Controller::busy() const
{
    return m_busy;
}

void Controller::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }
    m_busy = busy;
    Q_EMIT busyChanged();
}

Connection *Controller::connection() const
{
    if (m_connection.isNull())
        return nullptr;
    return m_connection;
}

void Controller::setConnection(Connection *connection)
{
    if (connection == m_connection)
        return;
    m_connection = connection;
    Q_EMIT connectionChanged();
}
