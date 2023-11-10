// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "controller.h"

#include <qt6keychain/keychain.h>

#include <KLocalizedString>
#include <KWindowConfig>

#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QNetworkProxy>
#include <QQuickTextDocument>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QTimer>

#include <signal.h>

#include <Quotient/accountregistry.h>
#include <Quotient/connection.h>
#include <Quotient/csapi/logout.h>
#include <Quotient/csapi/notifications.h>
#include <Quotient/eventstats.h>
#include <Quotient/jobs/downloadfilejob.h>
#include <Quotient/qt_connection_util.h>
#include <Quotient/user.h>

#include "neochatconfig.h"
#include "neochatroom.h"
#include "notificationsmanager.h"
#include "roommanager.h"
#include "windowcontroller.h"

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
#include "trayicon.h"
#elif !defined(Q_OS_ANDROID)
#include "trayicon_sni.h"
#endif

#ifdef HAVE_KUNIFIEDPUSH
#include <QCoro>
#include <Quotient/csapi/pusher.h>
#include <Quotient/networkaccessmanager.h>
#include <kunifiedpush/connector.h>
#endif

using namespace Quotient;
using namespace Qt::StringLiterals;

Controller::Controller(QObject *parent)
    : QObject(parent)
{
    Connection::setRoomType<NeoChatRoom>();

    setApplicationProxy();

#ifndef Q_OS_ANDROID
    setQuitOnLastWindowClosed();
    connect(NeoChatConfig::self(), &NeoChatConfig::SystemTrayChanged, this, &Controller::setQuitOnLastWindowClosed);
#endif

    QTimer::singleShot(0, this, [this] {
        invokeLogin();
    });

    QObject::connect(QGuiApplication::instance(), &QCoreApplication::aboutToQuit, QGuiApplication::instance(), [this] {
        delete m_trayIcon;
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

    static int oldAccountCount = 0;
    connect(&m_accountRegistry, &AccountRegistry::accountCountChanged, this, [this]() {
        if (m_accountRegistry.size() > oldAccountCount) {
            auto connection = dynamic_cast<NeoChatConnection *>(m_accountRegistry.accounts()[m_accountRegistry.size() - 1]);
            connect(connection, &NeoChatConnection::syncDone, this, [connection]() {
                NotificationsManager::instance().handleNotifications(connection);
            });
        }
        oldAccountCount = m_accountRegistry.size();
    });

#ifdef HAVE_KUNIFIEDPUSH
    auto connector = new KUnifiedPush::Connector(QStringLiteral("org.kde.neochat"));
    connect(connector, &KUnifiedPush::Connector::endpointChanged, this, &Controller::setupPushNotifications);

    connector->registerClient(i18n("Receiving push notifications"));
#endif
}

QCoro::Task<void> Controller::setupPushNotifications(QString endpoint)
{
#ifdef HAVE_KUNIFIEDPUSH
    while (!activeConnection()) {
        co_await qCoro(this, &Controller::activeConnectionChanged);
    }

    QUrl gatewayEndpoint(endpoint);
    gatewayEndpoint.setPath(QStringLiteral("/_matrix/push/v1/notify"));

    QNetworkRequest checkGateway(gatewayEndpoint);
    auto reply = co_await NetworkAccessManager::instance()->get(checkGateway);

    // We want to check if this UnifiedPush server has a Matrix gateway
    // This is because Matrix does not natively support UnifiedPush
    const QJsonObject replyJson = QJsonDocument::fromJson(reply->readAll()).object();

    if (replyJson["unifiedpush"_L1]["gateway"_L1].toString() == QStringLiteral("matrix")) {
        // FIXME: Currently hardcoded for ntfy URLs
        // We need to pass the ntfy topic as the pushkey. Is there a more generic way to handle this?
        const QUrl endpointUrl(endpoint);

        // Pop the slash off of the path
        const QString pushkey = endpointUrl.path().removeFirst();

        Controller::instance().activeConnection()->callApi<PostPusherJob>(
            pushkey,
            QStringLiteral("http"),
            QStringLiteral("org.kde.neochat"),
            QStringLiteral("NeoChat"),
            Controller::instance().activeConnection()->deviceId(),
            QString(), // FIXME: what is profileTag?
            QStringLiteral("en"),
            PostPusherJob::PusherData{QUrl::fromUserInput(gatewayEndpoint.toString()), QStringLiteral(" ")});

        qInfo() << "Registered for push notifications";
    } else {
        qWarning() << "There's no gateway, not setting up push notifications.";
    }
#else
    co_return;
#endif
}

Controller &Controller::instance()
{
    static Controller _instance;
    return _instance;
}

void Controller::toggleWindow()
{
    auto &instance = WindowController::instance();
    auto window = instance.window();
    if (window->isVisible()) {
        if (window->windowStates() & Qt::WindowMinimized) {
            window->showNormal();
            window->requestActivate();
        } else {
            window->close();
        }
    } else {
        instance.showAndRaiseWindow({});
        instance.window()->requestActivate();
    }
}

void Controller::addConnection(NeoChatConnection *c)
{
    Q_ASSERT_X(c, __FUNCTION__, "Attempt to add a null connection");

    m_accountRegistry.add(c);

    c->setLazyLoading(true);

    connect(c, &NeoChatConnection::syncDone, this, [this, c] {
        Q_EMIT syncDone();

        c->sync(30000);
        c->saveState();
    });
    connect(c, &NeoChatConnection::loggedOut, this, [this, c] {
        dropConnection(c);
    });

    connect(c, &NeoChatConnection::requestFailed, this, [this](BaseJob *job) {
        if (job->error() == BaseJob::UserConsentRequired) {
            Q_EMIT userConsentRequired(job->errorUrl());
        }
    });

    c->sync();

    Q_EMIT connectionAdded(c);
}

void Controller::dropConnection(NeoChatConnection *c)
{
    Q_ASSERT_X(c, __FUNCTION__, "Attempt to drop a null connection");

    m_accountRegistry.drop(c);
    Q_EMIT connectionDropped(c);
}

void Controller::invokeLogin()
{
    const auto accounts = SettingsGroup("Accounts"_ls).childGroups();
    QString id = NeoChatConfig::self()->activeConnection();
    for (const auto &accountId : accounts) {
        AccountSettings account{accountId};
        m_accountsLoading += accountId;
        Q_EMIT accountsLoadingChanged();
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
                    accessToken = QString::fromLatin1(accessTokenLoadingJob->binaryData());
                } else {
                    return;
                }

                auto connection = new NeoChatConnection(account.homeserver());
                connect(connection, &NeoChatConnection::connected, this, [this, connection, id] {
                    connection->loadState();
                    addConnection(connection);
                    m_accountsLoading.removeAll(connection->userId());
                    Q_EMIT accountsLoadingChanged();
                    if (connection->userId() == id) {
                        setActiveConnection(connection);
                        connectSingleShot(connection, &NeoChatConnection::syncDone, this, &Controller::initiated);
                    }
                });
                connect(connection, &NeoChatConnection::loginError, this, [this, connection](const QString &error, const QString &) {
                    if (error == "Unrecognised access token"_ls) {
                        Q_EMIT errorOccured(i18n("Login Failed: Access Token invalid or revoked"));
                        connection->logout(false);
                    } else if (error == "Connection closed"_ls) {
                        Q_EMIT errorOccured(i18n("Login Failed: %1", error));
                        // Failed due to network connection issue. This might happen when the homeserver is
                        // temporary down, or the user trying to re-launch NeoChat in a network that cannot
                        // connect to the homeserver. In this case, we don't want to do logout().
                    } else {
                        Q_EMIT errorOccured(i18n("Login Failed: %1", error));
                        connection->logout(true);
                    }
                    Q_EMIT initiated();
                });
                connect(connection, &NeoChatConnection::networkError, this, [this](const QString &error, const QString &, int, int) {
                    Q_EMIT errorOccured(i18n("Network Error: %1", error));
                });
                connection->assumeIdentity(account.userId(), accessToken);
            });
        }
    }
    if (accounts.isEmpty()) {
        Q_EMIT initiated();
    }
}

QKeychain::ReadPasswordJob *Controller::loadAccessTokenFromKeyChain(const AccountSettings &account)
{
    qDebug() << "Reading access token from the keychain for" << account.userId();
    auto job = new QKeychain::ReadPasswordJob(qAppName(), this);
    job->setKey(account.userId());

    // Handling of errors
    connect(job, &QKeychain::Job::finished, this, [this, job]() {
        if (job->error() == QKeychain::Error::NoError) {
            return;
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
        return false;
    }
    return true;
}

bool Controller::supportSystemTray() const
{
#ifdef Q_OS_ANDROID
    return false;
#else
    auto de = QString::fromLatin1(qgetenv("XDG_CURRENT_DESKTOP"));
    return de != QStringLiteral("GNOME") && de != QStringLiteral("Pantheon");
#endif
}

void Controller::setQuitOnLastWindowClosed()
{
#ifndef Q_OS_ANDROID
    if (NeoChatConfig::self()->systemTray()) {
        m_trayIcon = new TrayIcon(this);
        m_trayIcon->show();
        connect(m_trayIcon, &TrayIcon::toggleWindow, this, &Controller::toggleWindow);
    } else {
        if (m_trayIcon) {
            disconnect(m_trayIcon, &TrayIcon::toggleWindow, this, &Controller::toggleWindow);
            delete m_trayIcon;
            m_trayIcon = nullptr;
        }
    }
    QGuiApplication::setQuitOnLastWindowClosed(!NeoChatConfig::self()->systemTray());
#else
    return;
#endif
}

NeoChatConnection *Controller::activeConnection() const
{
    if (m_connection.isNull()) {
        return nullptr;
    }
    return m_connection;
}

void Controller::setActiveConnection(NeoChatConnection *connection)
{
    if (connection == m_connection) {
        return;
    }
    if (m_connection != nullptr) {
        disconnect(m_connection, &NeoChatConnection::syncError, this, nullptr);
        disconnect(m_connection, &NeoChatConnection::accountDataChanged, this, nullptr);
    }
    m_connection = connection;
    if (connection != nullptr) {
        NeoChatConfig::self()->setActiveConnection(connection->userId());
        connect(connection, &NeoChatConnection::requestFailed, this, [](BaseJob *job) {
            if (dynamic_cast<DownloadFileJob *>(job) && job->jsonData()["errcode"_ls].toString() == "M_TOO_LARGE"_ls) {
                RoomManager::instance().warning(i18n("File too large to download."), i18n("Contact your matrix server administrator for support."));
            }
        });
    } else {
        NeoChatConfig::self()->setActiveConnection(QString());
    }
    NeoChatConfig::self()->save();
    Q_EMIT activeConnectionChanged();
}

void Controller::saveWindowGeometry()
{
    WindowController::instance().saveGeometry();
}

// TODO: Remove in favor of RoomManager::joinRoom
void Controller::joinRoom(const QString &alias)
{
    if (!alias.contains(":"_ls)) {
        Q_EMIT errorOccured(i18n("The room id you are trying to join is not valid"));
        return;
    }

    const auto knownServer = alias.mid(alias.indexOf(":"_ls) + 1);
    RoomManager::instance().joinRoom(m_connection, alias, QStringList{knownServer});
}

void Controller::forceRefreshTextDocument(QQuickTextDocument *textDocument, QQuickItem *item)
{
    // HACK: Workaround bug QTBUG 93281
    connect(textDocument->textDocument(), SIGNAL(imagesLoaded()), item, SLOT(updateWholeDocument()));
}

void Controller::setApplicationProxy()
{
    NeoChatConfig *cfg = NeoChatConfig::self();
    QNetworkProxy proxy;

    // type match to ProxyType from neochatconfig.kcfg
    switch (cfg->proxyType()) {
    case 1: // HTTP
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(cfg->proxyHost());
        proxy.setPort(cfg->proxyPort());
        proxy.setUser(cfg->proxyUser());
        proxy.setPassword(cfg->proxyPassword());
        break;
    case 2: // SOCKS 5
        proxy.setType(QNetworkProxy::Socks5Proxy);
        proxy.setHostName(cfg->proxyHost());
        proxy.setPort(cfg->proxyPort());
        proxy.setUser(cfg->proxyUser());
        proxy.setPassword(cfg->proxyPassword());
        break;
    case 0: // System Default
    default:
        // do nothing
        break;
    }

    QNetworkProxy::setApplicationProxy(proxy);
}

bool Controller::isFlatpak() const
{
#ifdef NEOCHAT_FLATPAK
    return true;
#else
    return false;
#endif
}

AccountRegistry &Controller::accounts()
{
    return m_accountRegistry;
}

#include "moc_controller.cpp"
