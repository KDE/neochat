// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "controller.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <qt5keychain/keychain.h>
#else
#include <qt6keychain/keychain.h>
#endif

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KWindowConfig>
#ifdef HAVE_WINDOWSYSTEM
#include <KWindowEffects>
#endif

#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImageReader>
#include <QNetworkProxy>
#include <QQuickTextDocument>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QTimer>

#include <signal.h>

#include <accountregistry.h>

#include <connection.h>
#include <csapi/content-repo.h>
#include <csapi/logout.h>
#include <csapi/profile.h>
#include <jobs/downloadfilejob.h>
#include <qt_connection_util.h>
#include <csapi/notifications.h>
#include <eventstats.h>

#include "neochatconfig.h"
#include "neochatroom.h"
#include "neochatuser.h"
#include "notificationsmanager.h"
#include "roommanager.h"
#include "windowcontroller.h"

#include <accountregistry.h>

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

    setApplicationProxy();

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

    connectUntil(&Accounts, &AccountRegistry::rowsInserted, this, [this]() {
        if (auto *connection = Accounts.get(NeoChatConfig::self()->activeConnection())) {
            connectSingleShot(connection, &Connection::loadedRoomState, this, [this, connection]() {
                setActiveConnection(connection);
            });
            return true;
        }
        return false;
    });

    connect(&Accounts, &AccountRegistry::rowsRemoved, this, [this]() {
        if (!Accounts.isLoggedIn(NeoChatConfig::self()->activeConnection())) {
            if (Accounts.size() > 0) {
                setActiveConnection(Accounts.accounts().at(0));
            } else {
                setActiveConnection(nullptr);
            }
        }
    });

    QMetaObject::invokeMethod(&Accounts, &AccountRegistry::invokeLogin);

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

    connect(&Accounts, &AccountRegistry::accountCountChanged, this, &Controller::activeConnectionIndexChanged);

    static int oldAccountCount = 0;
    connect(&Accounts, &AccountRegistry::accountCountChanged, this, [this]() {
        if (Accounts.size() > oldAccountCount) {
            auto connection = Accounts.accounts()[Accounts.size() - 1];
            connect(connection, &Connection::syncDone, this, [this, connection]() {
                bool changes = false;
                for (const auto &room : connection->allRooms()) {
                    if (m_notificationCounts[room] != room->unreadStats().notableCount) {
                        m_notificationCounts[room] = room->unreadStats().notableCount;
                        changes = true;
                    }
                }
                if (changes) {
                    handleNotifications(connection);
                }
            });
        }
        oldAccountCount = Accounts.size();
    });
}

void Controller::handleNotifications(QPointer<Quotient::Connection> connection)
{
    static QStringList initial;
    static QStringList oldNotifications;
    auto job = connection->callApi<GetNotificationsJob>();

    connect(job, &BaseJob::success, this, [job, connection]() {
        const auto notifications = job->jsonData()["notifications"].toArray();
        if (!initial.contains(connection->user()->id())) {
            initial.append(connection->user()->id());
            for (const auto &n : notifications) {
                oldNotifications += n.toObject()["event"].toObject()["event_id"].toString();
            }
            return;
        }
        for (const auto &n : notifications) {
            const auto notification = n.toObject();
            if (notification["read"].toBool()) {
                oldNotifications.removeOne(notification["event"].toObject()["event_id"].toString());
                continue;
            }
            if (oldNotifications.contains(notification["event"].toObject()["event_id"].toString())) {
                continue;
            }
            oldNotifications += notification["event"].toObject()["event_id"].toString();
            auto room = connection->room(notification["room_id"].toString());

            // If room exists, room is NOT active OR the application is NOT active, show notification
            if (room
                && !(RoomManager::instance().currentRoom() && room->id() == RoomManager::instance().currentRoom()->id()
                     && QGuiApplication::applicationState() == Qt::ApplicationActive)) {
                // The room might have been deleted (for example rejected invitation).
                auto sender = room->user(notification["event"].toObject()["sender"].toString());

                QString body;

                if (notification["event"].toObject()["type"].toString() == "org.matrix.msc3381.poll.start") {
                    body = notification["event"]
                               .toObject()["content"]
                               .toObject()["org.matrix.msc3381.poll.start"]
                               .toObject()["question"]
                               .toObject()["body"]
                               .toString();
                } else {
                    body = notification["event"].toObject()["content"].toObject()["body"].toString();
                }

                if (notification["event"]["type"] == "m.room.encrypted") {
#ifdef Quotient_E2EE_ENABLED
                    auto decrypted = connection->decryptNotification(notification);
                    body = decrypted["content"].toObject()["body"].toString();
#endif
                    if (body.isEmpty()) {
                        body = i18n("Encrypted Message");
                    }
                }

                QImage avatar_image;
                if (!sender->avatarUrl(room).isEmpty()) {
                    avatar_image = sender->avatar(128, room);
                } else {
                    avatar_image = room->avatar(128);
                }
                NotificationsManager::instance().postNotification(dynamic_cast<NeoChatRoom *>(room),
                                                                  sender->displayname(room),
                                                                  body,
                                                                  avatar_image,
                                                                  notification["event"].toObject()["event_id"].toString(),
                                                                  true);
            }
        }
    });
}

Controller &Controller::instance()
{
    static Controller _instance;
    return _instance;
}

void Controller::showWindow()
{
    WindowController::instance().showAndRaiseWindow(QString());
}

void Controller::changeAvatar(Connection *conn, const QUrl &localFile)
{
    auto job = conn->uploadFile(localFile.toLocalFile());
    if (isJobPending(job)) {
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
    auto *job = connection->callApi<NeochatChangePasswordJob>(newPassword, false);
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
            auto *innerJob = connection->callApi<NeochatChangePasswordJob>(newPassword, false, authData);
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
        connection->callApi<SetAvatarUrlJob>(localUser->id(), avatarSource);
        return true;
    }
    if (QImageReader(decoded).read().isNull()) {
        return false;
    } else {
        return localUser->setAvatar(decoded);
    }
}

NeochatChangePasswordJob::NeochatChangePasswordJob(const QString &newPassword, bool logoutDevices, const Omittable<QJsonObject> &auth)
    : BaseJob(HttpVerb::Post, QStringLiteral("ChangePasswordJob"), "/_matrix/client/r0/account/password")
{
    QJsonObject _data;
    addParam<>(_data, QStringLiteral("new_password"), newPassword);
    addParam<IfNotEmpty>(_data, QStringLiteral("logout_devices"), logoutDevices);
    addParam<IfNotEmpty>(_data, QStringLiteral("auth"), auth);
    setRequestData(_data);
}

NeochatDeleteDeviceJob::NeochatDeleteDeviceJob(const QString &deviceId, const Omittable<QJsonObject> &auth)
    : Quotient::BaseJob(HttpVerb::Delete, QStringLiteral("DeleteDeviceJob"), QStringLiteral("/matrix/client/r0/devices/%1").arg(deviceId).toLatin1())
{
    QJsonObject _data;
    addParam<IfNotEmpty>(_data, QStringLiteral("auth"), auth);
    setRequestData(std::move(_data));
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
        disconnect(m_connection, &Connection::syncError, this, nullptr);
    }
    m_connection = connection;
    if (connection != nullptr) {
        NeoChatConfig::self()->setActiveConnection(connection->userId());
        connect(connection, &Connection::networkError, this, [this]() {
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
        connect(connection, &Connection::requestFailed, this, [=](BaseJob *job) {
            if (dynamic_cast<DownloadFileJob *>(job) && job->jsonData()["errcode"].toString() == "M_TOO_LARGE"_ls) {
                RoomManager::instance().warning(i18n("File too large to download."), i18n("Contact your matrix server administrator for support."));
            }
        });
    } else {
        NeoChatConfig::self()->setActiveConnection(QString());
    }
    NeoChatConfig::self()->save();
    Q_EMIT activeConnectionChanged();
    Q_EMIT activeConnectionIndexChanged();
}

void Controller::saveWindowGeometry()
{
    WindowController::instance().saveGeometry();
}

void Controller::createRoom(const QString &name, const QString &topic)
{
    auto createRoomJob = m_connection->createRoom(Connection::PublishRoom, "", name, topic, QStringList());
    connect(createRoomJob, &CreateRoomJob::failure, this, [this, createRoomJob] {
        Q_EMIT errorOccured(i18n("Room creation failed: \"%1\"", createRoomJob->errorString()));
    });
    connectSingleShot(
        this,
        &Controller::roomAdded,
        this,
        [](NeoChatRoom *room) {
            RoomManager::instance().enterRoom(room);
        },
        Qt::QueuedConnection);
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
        const auto &room = static_cast<NeoChatRoom *>(activeConnection()->room(existing.value(user)));
        if (room) {
            RoomManager::instance().enterRoom(room);
            return;
        }
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

bool Controller::hasWindowSystem() const
{
#ifdef HAVE_WINDOWSYSTEM
    return true;
#else
    return false;
#endif
}

QString Controller::plainText(QQuickTextDocument *document) const
{
    return document->textDocument()->toPlainText();
}

bool Controller::encryptionSupported() const
{
    return Quotient::encryptionSupported();
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

int Controller::activeConnectionIndex() const
{
    auto result = std::find_if(Accounts.accounts().begin(), Accounts.accounts().end(), [this](const auto &it) {
        return it == m_connection;
    });
    return result - Accounts.accounts().begin();
}

bool Controller::isFlatpak() const
{
#ifdef NEOCHAT_FLATPAK
    return true;
#else
    return false;
#endif
}
