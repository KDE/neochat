// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include <QCommandLineParser>
#include <QDebug>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QIcon>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlNetworkAccessManagerFactory>
#include <QQuickStyle>
#include <QQuickWindow>

#ifdef Q_OS_ANDROID
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#include <KAboutData>
#ifdef HAVE_KDBUSADDONS
#include <KDBusService>
#endif
#ifdef HAVE_WINDOWSYSTEM
#include <kwindowsystem_version.h>
#include <KWindowSystem>
#endif
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KWindowConfig>

#include "neochat-version.h"

#ifdef QUOTIENT_07
#include "accountregistry.h"
#else
#include "neochataccountregistry.h"
#endif

#include "actionshandler.h"
#include "blurhashimageprovider.h"
#include "chatboxhelper.h"
#include "chatdocumenthandler.h"
#include "clipboard.h"
#include "collapsestateproxymodel.h"
#include "commandmodel.h"
#include "controller.h"
#include "csapi/joining.h"
#include "csapi/leaving.h"
#include "customemojimodel.h"
#include "devicesmodel.h"
#include "emojimodel.h"
#include "filetypesingleton.h"
#include "joinrulesevent.h"
#include "login.h"
#include "matriximageprovider.h"
#include "messageeventmodel.h"
#include "messagefiltermodel.h"
#include "neochatconfig.h"
#include "neochatroom.h"
#include "neochatuser.h"
#include "networkaccessmanager.h"
#include "notificationsmanager.h"
#include "publicroomlistmodel.h"
#include "roomlistmodel.h"
#include "roommanager.h"
#include "sortfilterroomlistmodel.h"
#include "spellcheckhighlighter.h"
#include "userdirectorylistmodel.h"
#include "userlistmodel.h"
#include "webshortcutmodel.h"
#include <room.h>
#ifdef HAVE_COLORSCHEME
#include "colorschemer.h"
#endif

#ifdef QT_DBUS_LIB
#include "runner.h"
#include <QDBusConnection>
#endif

using namespace Quotient;

class NetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
    QNetworkAccessManager *create(QObject *) override
    {
        return NetworkAccessManager::instance();
    }
};

#ifdef HAVE_WINDOWSYSTEM
static void raiseWindow(QWindow *window)
{
#if KWINDOWSYSTEM_VERSION >= QT_VERSION_CHECK(5, 91, 0)
    KWindowSystem::updateStartupId(window);
#else
    if (KWindowSystem::isPlatformWayland()) {
        KWindowSystem::setCurrentXdgActivationToken(qEnvironmentVariable("XDG_ACTIVATION_TOKEN"));
    }
#endif
    KWindowSystem::activateWindow(window->winId());
    window->raise();
}
#endif

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif
int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QNetworkProxyFactory::setUseSystemConfiguration(true);

#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.breeze"));
#else
    QIcon::setFallbackThemeName("breeze");
    QApplication app(argc, argv);
    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
#endif

#ifdef Q_OS_WINDOWS
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    QApplication::setStyle(QStringLiteral("breeze"));
    auto font = app.font();
    font.setPointSize(10);
    app.setFont(font);
#endif

    QGuiApplication::setOrganizationName("KDE");

    KAboutData about(QStringLiteral("neochat"),
                     i18n("NeoChat"),
                     QStringLiteral(NEOCHAT_VERSION_STRING),
                     i18n("Matrix client"),
                     KAboutLicense::GPL_V3,
                     i18n("© 2018-2020 Black Hat, 2020-2021 KDE Community"));
    about.addAuthor(i18n("Black Hat"), QString(), QStringLiteral("bhat@encom.eu.org"));
    about.addAuthor(i18n("Carl Schwan"), QString(), QStringLiteral("carl@carlschwan.eu"));
    about.addAuthor(i18n("Tobias Fella"), QString(), QStringLiteral("fella@posteo.de"));
    about.setOrganizationDomain("kde.org");
    about.setBugAddress("https://invent.kde.org/network/neochat/issues");

    about.addComponent(QStringLiteral("libQuotient"),
                       i18n("A Qt5 library to write cross-platform clients for Matrix"),
                       QString(),
                       QStringLiteral("https://github.com/quotient-im/libquotient"),
                       KAboutLicense::LGPL_V2_1);

    KAboutData::setApplicationData(about);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.neochat")));


#ifdef NEOCHAT_FLATPAK
    // Copy over the included FontConfig configuration to the
    // app's config dir:
    QFile::copy("/app/etc/fonts/conf.d/99-noto-mono-color-emoji.conf", "/var/config/fontconfig/conf.d/99-noto-mono-color-emoji.conf");
#endif

    Clipboard clipboard;
    auto config = NeoChatConfig::self();
    FileTypeSingleton fileTypeSingleton;

    Login *login = new Login();
    ChatBoxHelper chatBoxHelper;

#ifdef HAVE_COLORSCHEME
    ColorSchemer colorScheme;
    qmlRegisterSingletonInstance<ColorSchemer>("org.kde.neochat", 1, 0, "ColorSchemer", &colorScheme);
    if (!config->colorScheme().isEmpty()) {
        colorScheme.apply(config->colorScheme());
    }
#endif

    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "Controller", &Controller::instance());
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "Clipboard", &clipboard);
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "Config", config);
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "RoomManager", &RoomManager::instance());
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "FileType", &fileTypeSingleton);
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "LoginHelper", login);
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "ChatBoxHelper", &chatBoxHelper);
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "EmojiModel", new EmojiModel(&app));
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "CommandModel", new CommandModel(&app));
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "AccountRegistry", &Quotient::AccountRegistry::instance());
    qmlRegisterType<ActionsHandler>("org.kde.neochat", 1, 0, "ActionsHandler");
    qmlRegisterType<ChatDocumentHandler>("org.kde.neochat", 1, 0, "ChatDocumentHandler");
    qmlRegisterType<SpellcheckHighlighter>("org.kde.neochat", 1, 0, "SpellcheckHighlighter");
    qmlRegisterType<RoomListModel>("org.kde.neochat", 1, 0, "RoomListModel");
    qmlRegisterType<KWebShortcutModel>("org.kde.neochat", 1, 0, "WebShortcutModel");
    qmlRegisterType<UserListModel>("org.kde.neochat", 1, 0, "UserListModel");
    qmlRegisterType<CustomEmojiModel>("org.kde.neochat", 1, 0, "CustomEmojiModel");
    qmlRegisterType<MessageEventModel>("org.kde.neochat", 1, 0, "MessageEventModel");
    qmlRegisterType<CollapseStateProxyModel>("org.kde.neochat", 1, 0, "CollapseStateProxyModel");
    qmlRegisterType<MessageFilterModel>("org.kde.neochat", 1, 0, "MessageFilterModel");
    qmlRegisterType<PublicRoomListModel>("org.kde.neochat", 1, 0, "PublicRoomListModel");
    qmlRegisterType<UserDirectoryListModel>("org.kde.neochat", 1, 0, "UserDirectoryListModel");
    qmlRegisterType<SortFilterRoomListModel>("org.kde.neochat", 1, 0, "SortFilterRoomListModel");
    qmlRegisterType<DevicesModel>("org.kde.neochat", 1, 0, "DevicesModel");
    qmlRegisterUncreatableType<RoomMessageEvent>("org.kde.neochat", 1, 0, "RoomMessageEvent", "ENUM");
    qmlRegisterUncreatableType<NeoChatRoomType>("org.kde.neochat", 1, 0, "NeoChatRoomType", "ENUM");
    qmlRegisterUncreatableType<UserType>("org.kde.neochat", 1, 0, "UserType", "ENUM");

    qRegisterMetaType<User *>("User*");
    qRegisterMetaType<User *>("const User*");
    qRegisterMetaType<User *>("const Quotient::User*");
    qRegisterMetaType<Room *>("Room*");
    qRegisterMetaType<Connection *>("Connection*");
    qRegisterMetaType<MessageEventType>("MessageEventType");
    qRegisterMetaType<NeoChatRoom *>("NeoChatRoom*");
    qRegisterMetaType<NeoChatUser *>("NeoChatUser*");
    qRegisterMetaType<GetRoomEventsJob *>("GetRoomEventsJob*");
    qRegisterMetaType<QMimeType>("QMimeType");

#ifdef HAVE_WINDOWSYSTEM
    qmlRegisterSingletonType<KWindowSystem>("org.kde.kwindowsystem.private", 1, 0, "KWindowSystem", [](QQmlEngine *, QJSEngine *) -> QObject * {
        return KWindowSystem::self();
    });
#endif

    qRegisterMetaTypeStreamOperators<Emoji>();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    KLocalizedString::setApplicationDomain("neochat");
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);
    engine.setNetworkAccessManagerFactory(new NetworkAccessManagerFactory());

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Client for the matrix communication protocol"));
    parser.addPositionalArgument(QStringLiteral("urls"), i18n("Supports appstream: url scheme"));

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    Controller::instance().setAboutData(about);

    engine.addImportPath("qrc:/imports");
    engine.addImageProvider(QLatin1String("mxc"), new MatrixImageProvider);
    engine.addImageProvider(QLatin1String("blurhash"), new BlurhashImageProvider);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (parser.positionalArguments().length() > 0) {
        RoomManager::instance().setUrlArgument(parser.positionalArguments()[0]);
    }

#ifdef QT_DBUS_LIB
    Runner runner;
    QDBusConnection::sessionBus().registerObject("/RoomRunner", &runner, QDBusConnection::ExportScriptableContents);
#endif

#ifdef HAVE_KDBUSADDONS
    KDBusService service(KDBusService::Unique);
    service.connect(&service,
                    &KDBusService::activateRequested,
                    &RoomManager::instance(),
                    [&engine](const QStringList &arguments, const QString &workingDirectory) {
                        Q_UNUSED(workingDirectory);

                        // Raise windows
                        const auto rootObjects = engine.rootObjects();
                        for (auto obj : rootObjects) {
                            auto view = qobject_cast<QQuickWindow *>(obj);
                            if (view) {
                                view->show();
                                raiseWindow(view);
                                return;
                            }
                        }

                        // Open matrix uri
                        if (arguments.isEmpty()) {
                            return;
                        }
                        auto args = arguments;
                        args.removeFirst();
                        for (const auto &arg : args) {
                            RoomManager::instance().openResource(arg);
                        }
                    });
#endif
    const auto rootObjects = engine.rootObjects();
    for (auto obj : rootObjects) {
        auto view = qobject_cast<QQuickWindow *>(obj);
        if (view) {
            if (view->isVisible()) {
                Controller::instance().restoreWindowGeometry(view);
            }
            break;
        }
    }
    return app.exec();
}
