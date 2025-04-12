// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "controller.h"

#include <Quotient/connection.h>
#include <qt6keychain/keychain.h>

#include <KLocalizedString>

#include <QGuiApplication>
#include <QTimer>

#include <signal.h>

#include <Quotient/csapi/notifications.h>
#include <Quotient/events/roommemberevent.h>
#include <Quotient/qt_connection_util.h>
#include <Quotient/settings.h>

#include "models/actionsmodel.h"
#include "models/messagemodel.h"
#include "models/pushrulemodel.h"
#include "neochatconfig.h"
#include "neochatconnection.h"
#include "neochatroom.h"
#include "notificationsmanager.h"
#include "proxycontroller.h"

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
#include "trayicon.h"
#elif !defined(Q_OS_ANDROID)
#include "trayicon_sni.h"
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 6, 0)
#ifndef Q_OS_ANDROID
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#endif
#endif

#ifdef HAVE_KUNIFIEDPUSH
#include <kunifiedpush/connector.h>
#endif

bool testMode = false;

using namespace Quotient;

Controller::Controller(QObject *parent)
    : QObject(parent)
{
    Connection::setRoomType<NeoChatRoom>();

    Connection::setDirectChatEncryptionDefault(NeoChatConfig::preferUsingEncryption());
    connect(NeoChatConfig::self(), &NeoChatConfig::PreferUsingEncryptionChanged, this, [] {
        Connection::setDirectChatEncryptionDefault(NeoChatConfig::preferUsingEncryption());
    });

    NeoChatConnection::setGlobalUrlPreviewDefault(NeoChatConfig::showLinkPreview());
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowLinkPreviewChanged, this, [this] {
        NeoChatConnection::setGlobalUrlPreviewDefault(NeoChatConfig::showLinkPreview());
        Q_EMIT globalUrlPreviewDefaultChanged();
    });

    NeoChatConnection::setKeywordPushRuleDefault(static_cast<PushRuleAction::Action>(NeoChatConfig::keywordPushRuleDefault()));
    connect(NeoChatConfig::self(), &NeoChatConfig::KeywordPushRuleDefaultChanged, this, [] {
        NeoChatConnection::setKeywordPushRuleDefault(static_cast<PushRuleAction::Action>(NeoChatConfig::keywordPushRuleDefault()));
    });

    ActionsModel::setAllowQuickEdit(NeoChatConfig::allowQuickEdit());
    connect(NeoChatConfig::self(), &NeoChatConfig::AllowQuickEditChanged, this, []() {
        ActionsModel::setAllowQuickEdit(NeoChatConfig::allowQuickEdit());
    });

    MessageModel::setHiddenFilter([](const RoomEvent *event) -> bool {
        if (event->isStateEvent() && !NeoChatConfig::showStateEvent()) {
            return true;
        }
        if (auto roomMemberEvent = eventCast<const RoomMemberEvent>(event)) {
            if ((roomMemberEvent->isJoin() || roomMemberEvent->isLeave()) && !NeoChatConfig::showLeaveJoinEvent()) {
                return true;
            } else if (roomMemberEvent->isRename() && !roomMemberEvent->isJoin() && !roomMemberEvent->isLeave() && !NeoChatConfig::showRename()) {
                return true;
            } else if (roomMemberEvent->isAvatarUpdate() && !roomMemberEvent->isJoin() && !roomMemberEvent->isLeave() && !NeoChatConfig::showAvatarUpdate()) {
                return true;
            }
        }
        return false;
    });

    ProxyController::instance().setApplicationProxy();

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
        c->assumeIdentity(u"@user:localhost:1234"_s, u"device_1234"_s, u"token_1234"_s);
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
            connect(
                connection,
                &NeoChatConnection::syncDone,
                this,
                [this, connection] {
                    if (!m_endpoint.isEmpty()) {
                        connection->setupPushNotifications(m_endpoint);
                    }
                },
                Qt::SingleShotConnection);
        }
        oldAccountCount = m_accountRegistry.size();
    });

#ifdef HAVE_KUNIFIEDPUSH
    auto connector = new KUnifiedPush::Connector(u"org.kde.neochat"_s);
    connect(connector, &KUnifiedPush::Connector::endpointChanged, this, [this](const QString &endpoint) {
        m_endpoint = endpoint;
        for (auto &quotientConnection : m_accountRegistry) {
            auto connection = dynamic_cast<NeoChatConnection *>(quotientConnection);
            connection->setupPushNotifications(endpoint);
        }
    });

    connector->registerClient(
        i18nc("The reason for using push notifications, as in: '[Push notifications are used for] Receiving notifications for new messages'",
              "Receiving notifications for new messages"));

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
        if (accounts().count() > 1) {
            // Only set the connection if the account being logged out is currently active
            if (c == activeConnection()) {
                setActiveConnection(dynamic_cast<NeoChatConnection *>(accounts().accounts()[0]));
            }
        } else {
            setActiveConnection(nullptr);
        }

        dropConnection(c);
    });
    connect(c, &NeoChatConnection::badgeNotificationCountChanged, this, &Controller::updateBadgeNotificationCount);
    connect(c, &NeoChatConnection::syncDone, this, [this, c]() {
        m_notificationsManager.handleNotifications(c);
    });
    connect(this, &Controller::globalUrlPreviewDefaultChanged, c, &NeoChatConnection::globalUrlPreviewEnabledChanged);

    c->sync();

    Q_EMIT connectionAdded(c);
}

void Controller::dropConnection(NeoChatConnection *c)
{
    Q_ASSERT_X(c, __FUNCTION__, "Attempt to drop a null connection");

    c->disconnect(this);
    c->disconnect(&m_notificationsManager);
    m_accountRegistry.drop(c);
    Q_EMIT connectionDropped(c);
}

void Controller::invokeLogin()
{
    const auto accounts = SettingsGroup("Accounts"_L1).childGroups();
    for (const auto &accountId : accounts) {
        AccountSettings account{accountId};
        m_accountsLoading += accountId;
        Q_EMIT accountsLoadingChanged();
        if (!account.homeserver().isEmpty()) {
            auto accessTokenLoadingJob = loadAccessTokenFromKeyChain(account.userId());
            connect(accessTokenLoadingJob, &QKeychain::Job::finished, this, [accountId, this, accessTokenLoadingJob](QKeychain::Job *) {
                AccountSettings account{accountId};
                QString accessToken;
                if (accessTokenLoadingJob->error() == QKeychain::Error::NoError) {
                    accessToken = QString::fromLatin1(accessTokenLoadingJob->binaryData());
                } else {
                    return;
                }

                auto connection = new NeoChatConnection(account.homeserver());
                m_connectionsLoading[accountId] = connection;
                connect(connection, &NeoChatConnection::connected, this, [this, connection, accountId] {
                    connection->loadState();
                    if (connection->allRooms().size() == 0 || connection->allRooms()[0]->currentState().get<RoomCreateEvent>()) {
                        addConnection(connection);
                        m_accountsLoading.removeAll(connection->userId());
                        m_connectionsLoading.remove(accountId);
                        Q_EMIT accountsLoadingChanged();
                    } else {
                        connect(
                            connection->allRooms()[0],
                            &Room::baseStateLoaded,
                            this,
                            [this, connection, accountId]() {
                                addConnection(connection);
                                m_accountsLoading.removeAll(connection->userId());
                                m_connectionsLoading.remove(accountId);
                                Q_EMIT accountsLoadingChanged();
                            },
                            Qt::SingleShotConnection);
                    }
                });
                connection->assumeIdentity(account.userId(), account.deviceId(), accessToken);
            });
        }
    }
}

QKeychain::ReadPasswordJob *Controller::loadAccessTokenFromKeyChain(const QString &userId)
{
    qDebug() << "Reading access token from the keychain for" << userId;
    auto job = new QKeychain::ReadPasswordJob(qAppName(), this);
    job->setKey(userId);

    // Handling of errors
    connect(job, &QKeychain::Job::finished, this, [this, job]() {
        if (job->error() == QKeychain::Error::NoError) {
            return;
        }

        switch (job->error()) {
        case QKeychain::EntryNotFound:
            Q_EMIT errorOccured(i18n("Access token wasn't found: Maybe it was deleted?"));
            break;
        case QKeychain::AccessDeniedByUser:
        case QKeychain::AccessDenied:
            Q_EMIT errorOccured(i18n("Access to keychain was denied: Please allow NeoChat to read the access token"));
            break;
        case QKeychain::NoBackendAvailable:
            Q_EMIT errorOccured(i18n("No keychain available: Please install a keychain, e.g. KWallet or GNOME keyring on Linux"));
            break;
        case QKeychain::OtherError:
            Q_EMIT errorOccured(i18n("Unable to read access token: %1", job->errorString()));
            break;
        default:
            break;
        }
    });
    job->start();

    return job;
}

void Controller::saveAccessTokenToKeyChain(const QString &userId, const QByteArray &accessToken)
{
    qDebug() << "Save the access token to the keychain for " << userId;
    auto job = new QKeychain::WritePasswordJob(qAppName());
    job->setAutoDelete(true);
    job->setKey(userId);
    job->setBinaryData(accessToken);
    connect(job, &QKeychain::WritePasswordJob::finished, this, [job]() {
        if (job->error()) {
            qWarning() << "Could not save access token to the keychain: " << qPrintable(job->errorString());
        }
    });
    job->start();
}

bool Controller::supportSystemTray() const
{
#ifdef Q_OS_ANDROID
    return false;
#else
    auto de = QString::fromLatin1(qgetenv("XDG_CURRENT_DESKTOP"));
    return de != u"GNOME"_s && de != u"Pantheon"_s;
#endif
}

void Controller::setQuitOnLastWindowClosed()
{
#ifndef Q_OS_ANDROID
    if (supportSystemTray() && NeoChatConfig::self()->systemTray()) {
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
        m_connection->disconnect(this);
        m_connection->disconnect(&m_notificationsManager);
    }

    m_connection = connection;

    if (m_connection != nullptr) {
        m_connection->refreshBadgeNotificationCount();
        updateBadgeNotificationCount(m_connection, m_connection->badgeNotificationCount());

        connect(m_connection, &NeoChatConnection::errorOccured, this, &Controller::errorOccured);
    }

    Q_EMIT activeConnectionChanged(m_connection);
}

void Controller::listenForNotifications()
{
#ifdef HAVE_KUNIFIEDPUSH
    auto connector = new KUnifiedPush::Connector(u"org.kde.neochat"_s);

    auto timer = new QTimer();
    connect(timer, &QTimer::timeout, qGuiApp, &QGuiApplication::quit);

    connect(connector, &KUnifiedPush::Connector::messageReceived, [timer](const QByteArray &data) {
        instance().m_notificationsManager.postPushNotification(data);
        timer->stop();
    });

    // Wait five seconds to see if we received any messages or this happened to be an erroneous activation.
    // Otherwise, messageReceived is never activated, and this daemon could stick around forever.
    timer->start(5000);

    connector->registerClient(i18n("Receiving push notifications"));
#endif
}

void Controller::clearInvitationNotification(const QString &roomId)
{
    m_notificationsManager.clearInvitationNotification(roomId);
}

void Controller::updateBadgeNotificationCount(NeoChatConnection *connection, int count)
{
    if (connection == m_connection) {
#if QT_VERSION < QT_VERSION_CHECK(6, 6, 0)
#ifndef Q_OS_ANDROID
        // copied from Telegram desktop
        const auto launcherUrl = "application://org.kde.neochat.desktop"_L1;
        // Gnome requires that count is a 64bit integer
        const qint64 counterSlice = std::min(count, 9999);
        QVariantMap dbusUnityProperties;

        if (counterSlice > 0) {
            dbusUnityProperties["count"_L1] = counterSlice;
            dbusUnityProperties["count-visible"_L1] = true;
        } else {
            dbusUnityProperties["count-visible"_L1] = false;
        }

        auto signal = QDBusMessage::createSignal("/com/canonical/unity/launcherentry/neochat"_L1, "com.canonical.Unity.LauncherEntry"_L1, "Update"_L1);

        signal.setArguments({launcherUrl, dbusUnityProperties});

        QDBusConnection::sessionBus().send(signal);
#endif // Q_OS_ANDROID
#else
        qGuiApp->setBadgeNumber(count);
#endif // QT_VERSION_CHECK(6, 6, 0)
    }
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

QString Controller::loadFileContent(const QString &path) const
{
    QUrl url(path);
    QFile file(url.isLocalFile() ? url.toLocalFile() : url.toString());
    file.open(QFile::ReadOnly);
    return QString::fromLatin1(file.readAll());
}

void Controller::setTestMode(bool test)
{
    testMode = test;
}

void Controller::removeConnection(const QString &userId)
{
    // When loadAccessTokenFromKeyChain() fails m_connectionsLoading won't have an
    // entry for it so we need to check both separately.
    if (m_accountsLoading.contains(userId)) {
        m_accountsLoading.removeAll(userId);
        Q_EMIT accountsLoadingChanged();
    }
    if (m_connectionsLoading.contains(userId) && m_connectionsLoading[userId]) {
        auto connection = m_connectionsLoading[userId];
        SettingsGroup("Accounts"_L1).remove(userId);
    }
}

void Controller::revertToDefaultConfig()
{
    const auto config = NeoChatConfig::self();
    config->setDefaults();
    config->save();
}

bool Controller::isImageShown(const QString &eventId)
{
    return m_shownImages.contains(eventId);
}

void Controller::markImageShown(const QString &eventId)
{
    m_shownImages.append(eventId);
}

#include "moc_controller.cpp"
