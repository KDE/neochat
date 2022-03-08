// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-3.0-only

#include "controller.h"

#include <qt5keychain/keychain.h>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KWindowConfig>
#ifdef HAVE_WINDOWSYSTEM
#include <KWindowEffects>
#include <KWindowSystem>
#endif

#include <QAuthenticator>
#include <QClipboard>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMovie>
#include <QNetworkConfigurationManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QQuickItem>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QSysInfo>
#include <QTimer>
#include <utility>

#include <signal.h>

#ifdef QUOTIENT_07
#include "accountregistry.h"
#else
#include "neochataccountregistry.h"
#endif

#include "csapi/account-data.h"
#include "csapi/content-repo.h"
#include "csapi/joining.h"
#include "csapi/logout.h"
#include "csapi/profile.h"
#include "events/eventcontent.h"
#include "neochatconfig.h"
#include "neochatroom.h"
#include "neochatuser.h"
#include "roommanager.h"
#include "settings.h"
#include "utils.h"

#include <KStandardShortcut>

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
#include "trayicon.h"
#elif !defined(Q_OS_ANDROID)
#include "trayicon_sni.h"
#endif

using namespace Quotient;

Controller::Controller(QObject *parent)
    : QObject(parent)
{
    Connection::setRoomType<NeoChatRoom>();
    Connection::setUserType<NeoChatUser>();

#ifndef Q_OS_ANDROID
    if (NeoChatConfig::self()->systemTray()) {
        m_trayIcon = new TrayIcon(this);
        m_trayIcon->show();
        connect(m_trayIcon, &TrayIcon::showWindow, this, &Controller::showWindow);
        QGuiApplication::setQuitOnLastWindowClosed(false);
    }
    connect(NeoChatConfig::self(), &NeoChatConfig::SystemTrayChanged, this, [this]() {
        if (NeoChatConfig::self()->systemTray()) {
            m_trayIcon = new TrayIcon(this);
            m_trayIcon->show();
            connect(m_trayIcon, &TrayIcon::showWindow, this, &Controller::showWindow);
        } else {
            disconnect(m_trayIcon, &TrayIcon::showWindow, this, &Controller::showWindow);
            delete m_trayIcon;
            m_trayIcon = nullptr;
        }
        QGuiApplication::setQuitOnLastWindowClosed(!NeoChatConfig::self()->systemTray());
    });
#endif

    QTimer::singleShot(0, this, [this] {
        invokeLogin();
    });

    QObject::connect(QGuiApplication::instance(), &QCoreApplication::aboutToQuit, QGuiApplication::instance(), [] {
        NeoChatConfig::self()->save();
    });

#ifndef Q_OS_WINDOWS
    // Setup Unix signal handlers
    const auto unixExitHandler = [](int /*sig*/) -> void {
        QCoreApplication::quit();
    };

    const int quitSignals[] = {SIGQUIT, SIGINT, SIGTERM, SIGHUP};

    sigset_t blockingMask;
    sigemptyset(&blockingMask);
    for (const auto sig : quitSignals) {
        sigaddset(&blockingMask, sig);
    }

    struct sigaction sa;
    sa.sa_handler = unixExitHandler;
    sa.sa_mask = blockingMask;
    sa.sa_flags = 0;

    for (auto sig : quitSignals) {
        sigaction(sig, &sa, nullptr);
    }
#endif
}

Controller::~Controller()
{
    for (auto c : AccountRegistry::instance().accounts()) {
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

void Controller::loginWithAccessToken(const QString &serverAddr, const QString &user, const QString &token, const QString &deviceName)
{
    if (user.isEmpty() || token.isEmpty()) {
        return;
    }

    QUrl serverUrl(serverAddr);

    auto conn = new Connection();
    if (serverUrl.isValid()) {
        conn->setHomeserver(serverUrl);
    }

    connect(conn, &Connection::connected, this, [this, conn, deviceName] {
        AccountSettings account(conn->userId());
        account.setKeepLoggedIn(true);
        account.setHomeserver(conn->homeserver());
        account.setDeviceId(conn->deviceId());
        account.setDeviceName(deviceName);
        if (!saveAccessTokenToKeyChain(account, conn->accessToken())) {
            qWarning() << "Couldn't save access token";
        }
        account.sync();
        addConnection(conn);
        setActiveConnection(conn);
    });
    connect(conn, &Connection::networkError, this, [this](QString error, const QString &, int, int) {
        Q_EMIT errorOccured(i18n("Network Error: %1", error));
    });
    conn->assumeIdentity(user, token, deviceName);
}

void Controller::logout(Connection *conn, bool serverSideLogout)
{
    if (!conn) {
        qCritical() << "Attempt to logout null connection";
        return;
    }

    SettingsGroup("Accounts").remove(conn->userId());
    QFile(accessTokenFileName(AccountSettings(conn->userId()))).remove();

    QKeychain::DeletePasswordJob job(qAppName());
    job.setAutoDelete(true);
    job.setKey(conn->userId());
    QEventLoop loop;
    QKeychain::DeletePasswordJob::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (conn == activeConnection() && AccountRegistry::instance().count() > 1) {
        setActiveConnection(AccountRegistry::instance().accounts()[0]);
    } else {
        setActiveConnection(nullptr);
    }
    if (!serverSideLogout) {
        return;
    }
    conn->logout();
}

void Controller::addConnection(Connection *c)
{
    Q_ASSERT_X(c, __FUNCTION__, "Attempt to add a null connection");

#ifndef QUOTIENT_07
    AccountRegistry::instance().add(c);
#endif

    c->setLazyLoading(true);

    connect(c, &Connection::syncDone, this, [this, c] {
        setBusy(false);

        Q_EMIT syncDone();

        c->sync(30000);
        c->saveState();
    });
    connect(c, &Connection::loggedOut, this, [this, c] {
        dropConnection(c);
    });

    connect(c, &Connection::requestFailed, this, [this](BaseJob *job) {
        if (job->error() == BaseJob::UserConsentRequiredError) {
            Q_EMIT userConsentRequired(job->errorUrl());
        }
    });

    setBusy(true);

    c->sync();

    Q_EMIT connectionAdded(c);
    Q_EMIT accountCountChanged();
}

void Controller::dropConnection(Connection *c)
{
    Q_ASSERT_X(c, __FUNCTION__, "Attempt to drop a null connection");

#ifndef QUOTIENT_07
    AccountRegistry::instance().drop(c);
#endif

    Q_EMIT connectionDropped(c);
    Q_EMIT accountCountChanged();
#ifndef QUOTIENT_07
    c->deleteLater();
#endif
}

void Controller::invokeLogin()
{
    const auto accounts = SettingsGroup("Accounts").childGroups();
    QString id = NeoChatConfig::self()->activeConnection();
    for (const auto &accountId : accounts) {
        AccountSettings account{accountId};
        if (id.isEmpty()) {
            // handle case where the account config is empty
            id = accountId;
        }
        if (!account.homeserver().isEmpty()) {
            auto accessTokenLoadingJob = loadAccessTokenFromKeyChain(account);
            connect(accessTokenLoadingJob, &QKeychain::Job::finished, this, [accountId, id, this, accessTokenLoadingJob](QKeychain::Job *) {
                AccountSettings account{accountId};
                QString accessToken;
                if (accessTokenLoadingJob->error() == QKeychain::Error::NoError) {
                    accessToken = accessTokenLoadingJob->binaryData();
                } else {
                    // No access token from the keychain, try token file
                    // TODO FIXME this code is racy since the file might have
                    // already been removed. But since the other code do a blocking
                    // dbus call, the probability are not high that it will happen.
                    // loadAccessTokenFromFile is also mostly legacy nowadays
                    accessToken = loadAccessTokenFromFile(account);
                    if (accessToken.isEmpty()) {
                        return;
                    }
                }

                auto connection = new Connection(account.homeserver());
                connect(connection, &Connection::connected, this, [this, connection, id] {
                    connection->loadState();
                    addConnection(connection);
                    if (connection->userId() == id) {
                        setActiveConnection(connection);
                        connectSingleShot(connection, &Connection::syncDone, this, &Controller::initiated);
                    }
                });
                connect(connection, &Connection::loginError, this, [this, connection](const QString &error, const QString &) {
                    if (error == "Unrecognised access token") {
                        Q_EMIT errorOccured(i18n("Login Failed: Access Token invalid or revoked"));
                        logout(connection, false);
                    } else {
                        Q_EMIT errorOccured(i18n("Login Failed: %1", error));
                        logout(connection, true);
                    }
                    Q_EMIT initiated();
                });
                connect(connection, &Connection::networkError, this, [this](const QString &error, const QString &, int, int) {
                    Q_EMIT errorOccured(i18n("Network Error: %1", error));
                });
                connection->assumeIdentity(account.userId(), accessToken, account.deviceId());
            });
        }
    }
    if (accounts.isEmpty()) {
        Q_EMIT initiated();
    }
}

QByteArray Controller::loadAccessTokenFromFile(const AccountSettings &account)
{
    QFile accountTokenFile{accessTokenFileName(account)};
    if (accountTokenFile.open(QFile::ReadOnly)) {
        if (accountTokenFile.size() < 1024) {
            return accountTokenFile.readAll();
        }

        qWarning() << "File" << accountTokenFile.fileName() << "is" << accountTokenFile.size() << "bytes long - too long for a token, ignoring it.";
    }
    qWarning() << "Could not open access token file" << accountTokenFile.fileName();

    return {};
}

QKeychain::ReadPasswordJob *Controller::loadAccessTokenFromKeyChain(const AccountSettings &account)
{
    qDebug() << "Reading access token from the keychain for" << account.userId();
    auto job = new QKeychain::ReadPasswordJob(qAppName(), this);
    job->setKey(account.userId());

    // Handling of errors
    connect(job, &QKeychain::Job::finished, this, [this, &account, job]() {
        if (job->error() == QKeychain::Error::NoError) {
            return;
        }
        if (job->error() == QKeychain::Error::EntryNotFound) {
            // no access token from the keychain, try token file
            auto accessToken = loadAccessTokenFromFile(account);
            if (!accessToken.isEmpty()) {
                qDebug() << "Migrating the access token from file to the keychain for " << account.userId();
                bool removed = false;
                bool saved = saveAccessTokenToKeyChain(account, accessToken);
                if (saved) {
                    QFile accountTokenFile{accessTokenFileName(account)};
                    removed = accountTokenFile.remove();
                }
                if (!(saved && removed)) {
                    qDebug() << "Migrating the access token from the file to the keychain "
                                "failed";
                }
                return;
            }
        }

        switch (job->error()) {
        case QKeychain::EntryNotFound:
            Q_EMIT globalErrorOccured(i18n("Access token wasn't found"), i18n("Maybe it was deleted?"));
            break;
        case QKeychain::AccessDeniedByUser:
        case QKeychain::AccessDenied:
            Q_EMIT globalErrorOccured(i18n("Access to keychain was denied."), i18n("Please allow NeoChat to read the access token"));
            break;
        case QKeychain::NoBackendAvailable:
            Q_EMIT globalErrorOccured(i18n("No keychain available."), i18n("Please install a keychain, e.g. KWallet or GNOME keyring on Linux"));
            break;
        case QKeychain::OtherError:
            Q_EMIT globalErrorOccured(i18n("Unable to read access token"), job->errorString());
            break;
        default:
            break;
        }
    });
    job->start();

    return job;
}

bool Controller::saveAccessTokenToFile(const AccountSettings &account, const QByteArray &accessToken)
{
    // (Re-)Make a dedicated file for access_token.
    QFile accountTokenFile{accessTokenFileName(account)};
    accountTokenFile.remove(); // Just in case

    auto fileDir = QFileInfo(accountTokenFile).dir();
    if (!((fileDir.exists() || fileDir.mkpath(".")) && accountTokenFile.open(QFile::WriteOnly))) {
        Q_EMIT errorOccured("I/O Denied: Cannot save access token.");
    } else {
        accountTokenFile.write(accessToken);
        return true;
    }
    return false;
}

bool Controller::saveAccessTokenToKeyChain(const AccountSettings &account, const QByteArray &accessToken)
{
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
    return true;
}

void Controller::changeAvatar(Connection *conn, const QUrl &localFile)
{
    auto job = conn->uploadFile(localFile.toLocalFile());
#ifdef QUOTIENT_07
    if (isJobPending(job)) {
#else
    if (isJobRunning(job)) {
#endif
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

void Controller::setAboutData(const KAboutData &aboutData)
{
    m_aboutData = aboutData;
    Q_EMIT aboutDataChanged();
}

KAboutData Controller::aboutData() const
{
    return m_aboutData;
}

bool Controller::supportSystemTray() const
{
#ifdef Q_OS_ANDROID
    return false;
#else
    QString de = getenv("XDG_CURRENT_DESKTOP");
    return de != QStringLiteral("GNOME") && de != QStringLiteral("Pantheon");
#endif
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

bool Controller::setAvatar(Connection *connection, const QUrl &avatarSource)
{
    User *localUser = connection->user();
    QString decoded = avatarSource.path();
    if (decoded.isEmpty()) {
#ifdef QUOTIENT_07
        connection->callApi<SetAvatarUrlJob>(localUser->id(), avatarSource);
#else
        connection->callApi<SetAvatarUrlJob>(localUser->id(), QString());
#endif
        return true;
    }
    if (QImageReader(decoded).read().isNull()) {
        return false;
    } else {
        return localUser->setAvatar(decoded);
    }
}

NeochatChangePasswordJob::NeochatChangePasswordJob(const QString &newPassword, bool logoutDevices, const Omittable<QJsonObject> &auth)
#ifdef QUOTIENT_07
    : BaseJob(HttpVerb::Post, QStringLiteral("ChangePasswordJob"), "/_matrix/client/r0/account/password")
#else
    : BaseJob(HttpVerb::Post, QStringLiteral("ChangePasswordJob"), QStringLiteral("/_matrix/client/r0/account/password"))
#endif
{
    QJsonObject _data;
    addParam<>(_data, QStringLiteral("new_password"), newPassword);
    addParam<IfNotEmpty>(_data, QStringLiteral("logout_devices"), logoutDevices);
    addParam<IfNotEmpty>(_data, QStringLiteral("auth"), auth);
    setRequestData(_data);
}

int Controller::accountCount() const
{
    return AccountRegistry::instance().count();
}

bool Controller::quitOnLastWindowClosed()
{
    return QGuiApplication::quitOnLastWindowClosed();
}

void Controller::setQuitOnLastWindowClosed(bool value)
{
    if (quitOnLastWindowClosed() != value) {
        QGuiApplication::setQuitOnLastWindowClosed(value);
        Q_EMIT quitOnLastWindowClosedChanged();
    }
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

Connection *Controller::activeConnection() const
{
    if (m_connection.isNull()) {
        return nullptr;
    }
    return m_connection;
}

void Controller::setActiveConnection(Connection *connection)
{
    if (connection == m_connection) {
        return;
    }
    if (m_connection != nullptr) {
        disconnect(connection, &Connection::syncError, this, nullptr);
    }
    m_connection = connection;
    if (connection != nullptr) {
        NeoChatConfig::self()->setActiveConnection(connection->userId());
        connect(connection, &Connection::networkError, this, [this](QString message, QString details, int retriesTaken, int nextRetryInMilliseconds) {
            if (!m_isOnline) {
                return;
            }
            m_isOnline = false;
            Q_EMIT isOnlineChanged(false);
        });
        connect(connection, &Connection::syncDone, this, [this] {
            if (m_isOnline) {
                return;
            }
            m_isOnline = true;
            Q_EMIT isOnlineChanged(true);
        });
    } else {
        NeoChatConfig::self()->setActiveConnection(QString());
    }
    NeoChatConfig::self()->save();
    Q_EMIT activeConnectionChanged();
}

void Controller::restoreWindowGeometry(QQuickWindow *window)
{
    KConfig dataResource("data", KConfig::SimpleConfig, QStandardPaths::AppDataLocation);
    KConfigGroup windowGroup(&dataResource, "Window");
    KWindowConfig::restoreWindowSize(window, windowGroup);
    KWindowConfig::restoreWindowPosition(window, windowGroup);
}

void Controller::saveWindowGeometry(QQuickWindow *window)
{
    KConfig dataResource("data", KConfig::SimpleConfig, QStandardPaths::AppDataLocation);
    KConfigGroup windowGroup(&dataResource, "Window");
    KWindowConfig::saveWindowPosition(window, windowGroup);
    KWindowConfig::saveWindowSize(window, windowGroup);
    dataResource.sync();
}

NeochatDeleteDeviceJob::NeochatDeleteDeviceJob(const QString &deviceId, const Omittable<QJsonObject> &auth)
#ifdef QUOTIENT_07
    : Quotient::BaseJob(HttpVerb::Delete, QStringLiteral("DeleteDeviceJob"), QStringLiteral("/_matrix/client/r0/devices/%1").arg(deviceId).toLatin1())
#else
    : Quotient::BaseJob(HttpVerb::Delete, QStringLiteral("DeleteDeviceJob"), QStringLiteral("/_matrix/client/r0/devices/%1").arg(deviceId))
#endif
{
    QJsonObject _data;
    addParam<IfNotEmpty>(_data, QStringLiteral("auth"), auth);
    setRequestData(std::move(_data));
}

void Controller::createRoom(const QString &name, const QString &topic)
{
    auto createRoomJob = m_connection->createRoom(Connection::PublishRoom, "", name, topic, QStringList());
    Quotient::CreateRoomJob::connect(createRoomJob, &CreateRoomJob::failure, [this, createRoomJob] {
        Q_EMIT errorOccured(i18n("Room creation failed: \"%1\"", createRoomJob->errorString()));
    });
}

bool Controller::isOnline() const
{
    return m_isOnline;
}

// TODO: Remove in favor of RoomManager::joinRoom
void Controller::joinRoom(const QString &alias)
{
    if (!alias.contains(":")) {
        Q_EMIT errorOccured(i18n("The room id you are trying to join is not valid"));
        return;
    }

    const auto knownServer = alias.mid(alias.indexOf(":") + 1);
    RoomManager::instance().joinRoom(m_connection, alias, QStringList{knownServer});
}

void Controller::openOrCreateDirectChat(NeoChatUser *user)
{
    const auto existing = activeConnection()->directChats();

    if (existing.contains(user)) {
        RoomManager::instance().enterRoom(static_cast<NeoChatRoom *>(activeConnection()->room(existing.value(user))));
        return;
    }
    activeConnection()->requestDirectChat(user);
}

QString Controller::formatByteSize(double size, int precision) const
{
    return QLocale().formattedDataSize(size, precision);
}

QString Controller::formatDuration(quint64 msecs, KFormat::DurationFormatOptions options) const
{
    return KFormat().formatDuration(msecs, options);
}

void Controller::setBlur(QQuickItem *item, bool blur)
{
#ifdef HAVE_WINDOWSYSTEM
    auto setWindows = [item, blur]() {
        auto reg = QRect(QPoint(0, 0), item->window()->size());
        KWindowEffects::enableBackgroundContrast(item->window(), blur, 1, 1, 1, reg);
        KWindowEffects::enableBlurBehind(item->window(), blur, reg);
    };

    disconnect(item->window(), &QQuickWindow::heightChanged, this, nullptr);
    disconnect(item->window(), &QQuickWindow::widthChanged, this, nullptr);
    connect(item->window(), &QQuickWindow::heightChanged, this, setWindows);
    connect(item->window(), &QQuickWindow::widthChanged, this, setWindows);
    setWindows();
#endif
}

void Controller::raiseWindow(QWindow *window)
{
#ifdef HAVE_WINDOWSYSTEM
    KWindowSystem::activateWindow(window->winId());
#endif
}

bool Controller::hasWindowSystem() const
{
#ifdef HAVE_WINDOWSYSTEM
    return true;
#else
    return false;
#endif
}
