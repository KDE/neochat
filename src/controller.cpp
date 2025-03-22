// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "controller.h"

#include <qt6keychain/keychain.h>

#include <KLocalizedString>

#include <QFile>
#include <QGuiApplication>
#include <QTimer>

#include <signal.h>

#include "neochatconfig.h"
#include "neochatconnection.h"
// #include "neochatroom.h"
// #include "notificationsmanager.h"
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

using namespace Integral;
using namespace Qt::Literals::StringLiterals;

Controller::Controller(QObject *parent)
    : QObject(parent)
{
    // Connection::setRoomType<NeoChatRoom>();

    ProxyController::instance().setApplicationProxy();

#ifndef Q_OS_ANDROID
    setQuitOnLastWindowClosed();
    connect(NeoChatConfig::self(), &NeoChatConfig::SystemTrayChanged, this, &Controller::setQuitOnLastWindowClosed);
#endif

    // if (!testMode) {
    //     QTimer::singleShot(0, this, [this] {
    //         invokeLogin();
    //     });
    // } else {
    //     auto c = new NeoChatConnection(this);
    //     c->assumeIdentity(u"@user:localhost:1234"_s, u"device_1234"_s, u"token_1234"_s);
    //     connect(c, &Connection::connected, this, [c, this]() {
    //         m_accountRegistry.add(c);
    //         c->syncLoop();
    //     });
    // }

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
    // connect(&m_accountRegistry, &AccountRegistry::accountCountChanged, this, [this]() {
    //     if (m_accountRegistry.size() > oldAccountCount) {
    //         auto connection = dynamic_cast<NeoChatConnection *>(m_accountRegistry.accounts()[m_accountRegistry.size() - 1]);
    //         connect(
    //             connection,
    //             &NeoChatConnection::syncDone,
    //             this,
    //             [this, connection] {
    //                 if (!m_endpoint.isEmpty()) {
    //                     connection->setupPushNotifications(m_endpoint);
    //                 }
    //             },
    //             Qt::SingleShotConnection);
    //     }
    //     oldAccountCount = m_accountRegistry.size();
    // });

#ifdef HAVE_KUNIFIEDPUSH
    auto connector = new KUnifiedPush::Connector(u"org.kde.neochat"_s);
    connect(connector, &KUnifiedPush::Connector::endpointChanged, this, [this](const QString &endpoint) {
        m_endpoint = endpoint;
        // for (auto &quotientConnection : m_accountRegistry) {
        //     auto connection = dynamic_cast<NeoChatConnection *>(quotientConnection);
        //     connection->setupPushNotifications(endpoint);
        // }
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
        // m_connection->disconnect(&m_notificationsManager);
    }

    m_connection = connection;

    if (m_connection != nullptr) {
        m_connection->refreshBadgeNotificationCount();
        updateBadgeNotificationCount(m_connection, m_connection->badgeNotificationCount());

        // connect(m_connection, &NeoChatConnection::errorOccured, this, &Controller::errorOccured);
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
        // instance().m_notificationsManager.postPushNotification(data);
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
    // m_notificationsManager.clearInvitationNotification(roomId);
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

Accounts &Controller::accounts()
{
    return m_accounts;
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

bool Controller::csSupported() const
{
    return true;
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
