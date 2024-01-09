// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "controller.h"

#include <qt6keychain/keychain.h>

#include <KLocalizedString>

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

#include "neochatconfig.h"
#include "neochatroom.h"
#include "notificationsmanager.h"
#include "roommanager.h"

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
#include "trayicon.h"
#elif !defined(Q_OS_ANDROID)
#include "trayicon_sni.h"
#endif

bool testMode = false;

using namespace Quotient;

Controller::Controller(QObject *parent)
    : QObject(parent)
{
    Connection::setRoomType<NeoChatRoom>();

    setApplicationProxy();

#ifndef Q_OS_ANDROID
    setQuitOnLastWindowClosed();
    connect(NeoChatConfig::self(), &NeoChatConfig::SystemTrayChanged, this, &Controller::setQuitOnLastWindowClosed);
#endif

    if (!testMode) {
        QTimer::singleShot(0, this, [this] {
            invokeLogin();
        });
    } else {
        auto c = new NeoChatConnection(this);
        c->assumeIdentity(QStringLiteral("@user:localhost:1234"), QStringLiteral("token_1234"));
        connect(c, &Connection::connected, this, [c, this]() {
            m_accountRegistry.add(c);
            c->syncLoop();
        });
    }

    QObject::connect(QGuiApplication::instance(), &QCoreApplication::aboutToQuit, QGuiApplication::instance(), [this] {
        delete m_trayIcon;
        NeoChatConfig::self()->save();
    });

#ifndef Q_OS_WINDOWS
    const auto unixExitHandler = [](int) -> void {
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
            connectSingleShot(connection, &NeoChatConnection::syncDone, this, [this, connection] {
                connection->setupPushNotifications(m_endpoint);
            });
        }
        oldAccountCount = m_accountRegistry.size();
    });

#ifdef HAVE_KUNIFIEDPUSH
    auto connector = new KUnifiedPush::Connector(QStringLiteral("org.kde.neochat"));
    connect(connector, &KUnifiedPush::Connector::endpointChanged, this, [this](const QString &endpoint) {
        m_endpoint = endpoint;
        for (auto &quotientConnection : m_accountRegistry) {
            auto connection = dynamic_cast<NeoChatConnection *>(quotientConnection);
            connection->setupPushNotifications(endpoint);
        }
    });

    connector->registerClient(i18n("Receiving push notifications"));

    m_endpoint = connector->endpoint();
#endif
}

Controller &Controller::instance()
{
    static Controller _instance;
    return _instance;
}

void Controller::addConnection(NeoChatConnection *c)
{
    Q_ASSERT_X(c, __FUNCTION__, "Attempt to add a null connection");

    m_accountRegistry.add(c);

    c->setLazyLoading(true);

    connect(c, &NeoChatConnection::syncDone, this, [c] {
        c->sync(30000);
        c->saveState();
    });
    connect(c, &NeoChatConnection::loggedOut, this, [this, c] {
        dropConnection(c);
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
    for (const auto &accountId : accounts) {
        AccountSettings account{accountId};
        m_accountsLoading += accountId;
        Q_EMIT accountsLoadingChanged();
        if (!account.homeserver().isEmpty()) {
            auto accessTokenLoadingJob = loadAccessTokenFromKeyChain(account);
            connect(accessTokenLoadingJob, &QKeychain::Job::finished, this, [accountId, this, accessTokenLoadingJob](QKeychain::Job *) {
                AccountSettings account{accountId};
                QString accessToken;
                if (accessTokenLoadingJob->error() == QKeychain::Error::NoError) {
                    accessToken = QString::fromLatin1(accessTokenLoadingJob->binaryData());
                } else {
                    return;
                }

                auto connection = new NeoChatConnection(account.homeserver());
                connect(connection, &NeoChatConnection::connected, this, [this, connection] {
                    connection->loadState();
                    addConnection(connection);
                    m_accountsLoading.removeAll(connection->userId());
                    Q_EMIT accountsLoadingChanged();
                });
                connect(connection, &NeoChatConnection::networkError, this, [this](const QString &error, const QString &, int, int) {
                    Q_EMIT errorOccured(i18n("Network Error: %1", error), {});
                });
                connection->assumeIdentity(account.userId(), accessToken);
            });
        }
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
            Q_EMIT errorOccured(i18n("Access token wasn't found"), i18n("Maybe it was deleted?"));
            break;
        case QKeychain::AccessDeniedByUser:
        case QKeychain::AccessDenied:
            Q_EMIT errorOccured(i18n("Access to keychain was denied."), i18n("Please allow NeoChat to read the access token"));
            break;
        case QKeychain::NoBackendAvailable:
            Q_EMIT errorOccured(i18n("No keychain available."), i18n("Please install a keychain, e.g. KWallet or GNOME keyring on Linux"));
            break;
        case QKeychain::OtherError:
            Q_EMIT errorOccured(i18n("Unable to read access token"), job->errorString());
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
    } else {
        if (m_trayIcon) {
            delete m_trayIcon;
            m_trayIcon = nullptr;
        }
    }
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
        connect(connection, &NeoChatConnection::requestFailed, this, [](BaseJob *job) {
            if (dynamic_cast<DownloadFileJob *>(job) && job->jsonData()["errcode"_ls].toString() == "M_TOO_LARGE"_ls) {
                RoomManager::instance().warning(i18n("File too large to download."), i18n("Contact your matrix server administrator for support."));
            }
        });
    }
    NeoChatConfig::self()->save();
    Q_EMIT activeConnectionChanged();
}

void Controller::forceRefreshTextDocument(QQuickTextDocument *textDocument, QQuickItem *item)
{
    // HACK: Workaround bug QTBUG 93281
    connect(textDocument->textDocument(), SIGNAL(imagesLoaded()), item, SLOT(updateWholeDocument()));
}

void Controller::listenForNotifications()
{
#ifdef HAVE_KUNIFIEDPUSH
    auto connector = new KUnifiedPush::Connector(QStringLiteral("org.kde.neochat"));

    auto timer = new QTimer();
    connect(timer, &QTimer::timeout, qGuiApp, &QGuiApplication::quit);

    connect(connector, &KUnifiedPush::Connector::messageReceived, [timer](const QByteArray &data) {
        NotificationsManager::instance().postPushNotification(data);
        timer->stop();
    });

    // Wait five seconds to see if we received any messages or this happened to be an erroneous activation.
    // Otherwise, messageReceived is never activated, and this daemon could stick around forever.
    timer->start(5000);

    connector->registerClient(i18n("Receiving push notifications"));
#endif
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

void Controller::setTestMode(bool test)
{
    testMode = test;
}
