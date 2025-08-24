// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "controller.h"

#include <Quotient/connection.h>

#include <KLocalizedString>

#include <QGuiApplication>
#include <QTimer>

#include <signal.h>

#include <Quotient/events/roommemberevent.h>
#include <Quotient/qt_connection_util.h>
#include <Quotient/settings.h>

#include "accountmanager.h"
#include "enums/roomsortparameter.h"
#include "general_logging.h"
#include "mediasizehelper.h"
#include "models/actionsmodel.h"
#include "models/messagemodel.h"
#include "models/roomlistmodel.h"
#include "models/roomtreemodel.h"
#include "neochatconfig.h"
#include "neochatconnection.h"
#include "neochatroom.h"
#include "notificationsmanager.h"
#include "proxycontroller.h"
#include "roommanager.h"

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

using namespace Quotient;

static std::function<bool(const Quotient::RoomEvent *)> hiddenEventFilter = [](const RoomEvent *event) -> bool {
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
};

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

    MessageModel::setHiddenFilter(hiddenEventFilter);
    RoomListModel::setHiddenFilter(hiddenEventFilter);
    RoomTreeModel::setHiddenFilter(hiddenEventFilter);
    NeoChatRoom::setHiddenFilter(hiddenEventFilter);

    MediaSizeHelper::setMaxSize(NeoChatConfig::mediaMaxWidth(), NeoChatConfig::mediaMaxHeight());
    connect(NeoChatConfig::self(), &NeoChatConfig::MediaMaxWidthChanged, this, []() {
        MediaSizeHelper::setMaxSize(NeoChatConfig::mediaMaxWidth(), NeoChatConfig::mediaMaxHeight());
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::MediaMaxHeightChanged, this, []() {
        MediaSizeHelper::setMaxSize(NeoChatConfig::mediaMaxWidth(), NeoChatConfig::mediaMaxHeight());
    });

    RoomSortParameter::setSortOrder(static_cast<RoomSortOrder::Order>(NeoChatConfig::sortOrder()));
    connect(NeoChatConfig::self(), &NeoChatConfig::SortOrderChanged, this, []() {
        RoomSortParameter::setSortOrder(static_cast<RoomSortOrder::Order>(NeoChatConfig::sortOrder()));
    });

    QList<RoomSortParameter::Parameter> configParamList;
    const auto intList = NeoChatConfig::customSortOrder();
    std::transform(intList.constBegin(), intList.constEnd(), std::back_inserter(configParamList), [](int param) {
        return static_cast<RoomSortParameter::Parameter>(param);
    });
    RoomSortParameter::setCustomSortOrder(configParamList);
    connect(NeoChatConfig::self(), &NeoChatConfig::CustomSortOrderChanged, this, []() {
        QList<RoomSortParameter::Parameter> configParamList;
        const auto intList = NeoChatConfig::customSortOrder();
        std::transform(intList.constBegin(), intList.constEnd(), std::back_inserter(configParamList), [](int param) {
            return static_cast<RoomSortParameter::Parameter>(param);
        });
        RoomSortParameter::setCustomSortOrder(configParamList);
    });

    ProxyController::instance().setApplicationProxy();

#ifndef Q_OS_ANDROID
    setQuitOnLastWindowClosed();
    connect(NeoChatConfig::self(), &NeoChatConfig::SystemTrayChanged, this, &Controller::setQuitOnLastWindowClosed);
#endif

    connect(QGuiApplication::instance(), &QCoreApplication::aboutToQuit, QGuiApplication::instance(), [this] {
#ifndef Q_OS_ANDROID
        delete m_trayIcon;
#endif
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

#ifdef HAVE_KUNIFIEDPUSH
    auto connector = new KUnifiedPush::Connector(u"org.kde.neochat"_s);
    connect(connector, &KUnifiedPush::Connector::endpointChanged, this, [this](const QString &endpoint) {
        if (!m_accountManager) {
            return;
        }

        m_endpoint = endpoint;
        for (auto &quotientConnection : m_accountManager->accounts()->accounts()) {
            auto connection = dynamic_cast<NeoChatConnection *>(quotientConnection);
            connection->setupPushNotifications(endpoint);
        }
    });

    connector->registerClient(
        i18nc("The reason for using push notifications, as in: '[Push notifications are used for] Receiving notifications for new messages'",
              "Receiving notifications for new messages"));

#endif
}

Controller &Controller::instance()
{
    static Controller _instance;
    return _instance;
}

void Controller::setAccountManager(AccountManager *manager)
{
    if (manager == m_accountManager) {
        return;
    }

    if (m_accountManager) {
        m_accountManager->disconnect(this);
    }

    m_accountManager = manager;

    if (!m_accountManager) {
        return;
    }

    connect(m_accountManager, &AccountManager::errorOccured, this, &Controller::errorOccured);
    connect(m_accountManager, &AccountManager::accountsLoadingChanged, this, &Controller::accountsLoadingChanged);
    connect(m_accountManager, &AccountManager::connectionAdded, this, &Controller::initConnection);
    connect(m_accountManager, &AccountManager::connectionDropped, this, &Controller::teardownConnection);
    connect(m_accountManager, &AccountManager::activeConnectionChanged, this, &Controller::initActiveConnection);
}

void Controller::initConnection(NeoChatConnection *connection)
{
    if (!connection) {
        return;
    }

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
    connect(connection, &NeoChatConnection::syncDone, this, [this, connection]() {
        m_notificationsManager.handleNotifications(connection);
    });
    connect(this, &Controller::globalUrlPreviewDefaultChanged, connection, &NeoChatConnection::globalUrlPreviewEnabledChanged);
    connect(connection, &NeoChatConnection::roomAboutToBeLeft, &RoomManager::instance(), &RoomManager::roomLeft);
    Q_EMIT connectionAdded(connection);
}

void Controller::teardownConnection(NeoChatConnection *connection)
{
    if (!connection) {
        return;
    }

    connection->disconnect(this);
    Q_EMIT connectionDropped(connection);
}

void Controller::initActiveConnection(NeoChatConnection *oldConnection, NeoChatConnection *newConnection)
{
    if (oldConnection) {
        oldConnection->disconnect(this);
    }

    if (newConnection) {
        connect(newConnection, &NeoChatConnection::errorOccured, this, &Controller::errorOccured);
        connect(newConnection, &NeoChatConnection::badgeNotificationCountChanged, this, &Controller::updateBadgeNotificationCount);
        newConnection->refreshBadgeNotificationCount();
    }
    Q_EMIT activeConnectionChanged(newConnection);
}

bool Controller::supportSystemTray() const
{
#ifdef Q_OS_ANDROID
    return false;
#else
    QStringList unsupportedPlatforms{u"GNOME"_s, u"Pantheon"_s};
    return !unsupportedPlatforms.contains(QString::fromLatin1(qgetenv("XDG_CURRENT_DESKTOP")));
#endif
}

void Controller::setQuitOnLastWindowClosed()
{
#ifndef Q_OS_ANDROID
    if (supportSystemTray() && NeoChatConfig::self()->systemTray()) {
        m_trayIcon = new TrayIcon(this);
        m_trayIcon->show();
    } else if (m_trayIcon) {
        delete m_trayIcon;
    }
#endif
}

NeoChatConnection *Controller::activeConnection() const
{
    if (!m_accountManager) {
        return nullptr;
    }
    return m_accountManager->activeConnection();
}

void Controller::setActiveConnection(NeoChatConnection *connection)
{
    if (!m_accountManager) {
        return;
    }
    m_accountManager->setActiveConnection(connection);
}

QStringList Controller::accountsLoading() const
{
    if (!m_accountManager) {
        return {};
    }
    return m_accountManager->accountsLoading();
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

void Controller::updateBadgeNotificationCount(int count)
{
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

bool Controller::isFlatpak() const
{
#ifdef NEOCHAT_FLATPAK
    return true;
#else
    return false;
#endif
}

AccountRegistry *Controller::accounts()
{
    return m_accountManager->accounts();
}

QString Controller::loadFileContent(const QString &path) const
{
    QUrl url(path);
    QFile file(url.isLocalFile() ? url.toLocalFile() : url.toString());
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(GENERAL) << "Failed to open file" << path;
        return {};
    }
    return QString::fromLatin1(file.readAll());
}

void Controller::removeConnection(const QString &userId)
{
    m_accountManager->dropConnection(userId);
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

void Controller::markImageHidden(const QString &eventId)
{
    m_shownImages.removeAll(eventId);
}

#include "moc_controller.cpp"
