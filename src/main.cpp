// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include <QCommandLineParser>
#include <QIcon>
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
#include <KWindowSystem>
#endif
#include <KLocalizedContext>
#include <KLocalizedString>

#include "neochat-version.h"

#ifdef QUOTIENT_07
#include <accountregistry.h>
#else
#include "neochataccountregistry.h"
#endif

#include <networkaccessmanager.h>
#include <room.h>
#include <util.h>

#include "actionshandler.h"
#include "blurhashimageprovider.h"
#include "chatdocumenthandler.h"
#include "clipboard.h"
#include "controller.h"
#include "filetypesingleton.h"
#include "joinrulesevent.h"
#include "linkpreviewer.h"
#include "login.h"
#include "matriximageprovider.h"
#include "models/collapsestateproxymodel.h"
#include "models/customemojimodel.h"
#include "models/devicesmodel.h"
#include "models/emojimodel.h"
#include "models/keywordnotificationrulemodel.h"
#include "models/messageeventmodel.h"
#include "models/messagefiltermodel.h"
#include "models/publicroomlistmodel.h"
#include "models/roomlistmodel.h"
#include "models/searchmodel.h"
#include "models/serverlistmodel.h"
#include "models/sortfilterroomlistmodel.h"
#include "models/sortfilterspacelistmodel.h"
#include "models/userdirectorylistmodel.h"
#include "models/userfiltermodel.h"
#include "models/userlistmodel.h"
#include "models/webshortcutmodel.h"
#include "neochatconfig.h"
#include "neochatroom.h"
#include "neochatuser.h"
#include "notificationsmanager.h"
#ifdef QUOTIENT_07
#include "pollhandler.h"
#endif
#include "roommanager.h"
#include "spacehierarchycache.h"
#include "urlhelper.h"
#include "windowcontroller.h"
#ifdef QUOTIENT_07
#include <keyverificationsession.h>
#endif
#ifdef HAVE_COLORSCHEME
#include "colorschemer.h"
#endif
#include "models/completionmodel.h"
#include "models/statemodel.h"
#include "neochatuser.h"

#ifdef HAVE_RUNNER
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

static QWindow *windowFromEngine(QQmlApplicationEngine *engine)
{
    const auto rootObjects = engine->rootObjects();
    auto *window = qobject_cast<QQuickWindow *>(rootObjects.first());
    Q_ASSERT(window);
    return window;
}

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
    QFont font(QStringLiteral("Segoe UI Emoji"));
    font.setPointSize(10);
    font.setHintingPreference(QFont::PreferNoHinting);
    app.setFont(font);
#endif
    KLocalizedString::setApplicationDomain("neochat");

    QGuiApplication::setOrganizationName("KDE");

    KAboutData about(QStringLiteral("neochat"),
                     i18n("NeoChat"),
                     QStringLiteral(NEOCHAT_VERSION_STRING),
                     i18n("Matrix client"),
                     KAboutLicense::GPL_V3,
                     i18n("© 2018-2020 Black Hat, 2020-2022 KDE Community"));
    about.addAuthor(i18n("Carl Schwan"), i18n("Maintainer"), QStringLiteral("carl@carlschwan.eu"), QStringLiteral("https://carlschwan.eu"));
    about.addAuthor(i18n("Tobias Fella"), i18n("Maintainer"), QStringLiteral("fella@posteo.de"), QStringLiteral("https://tobiasfella.de"));
    about.addAuthor(i18n("James Graham"), i18n("Maintainer"), QStringLiteral("james.h.graham@protonmail.com"));
    about.addCredit(i18n("Black Hat"), i18n("Original author of Spectral"), QStringLiteral("bhat@encom.eu.org"));
    about.addCredit(i18n("Alexey Rusakov"), i18n("Maintainer of Quotient"), QStringLiteral("Kitsune-Ral@users.sf.net"));
    about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    about.setOrganizationDomain("kde.org");

    about.addComponent(QStringLiteral("libQuotient"),
                       i18n("A Qt5 library to write cross-platform clients for Matrix"),
#ifdef QUOTIENT_07
                       i18nc("<version number> (built against <possibly different version number>)",
                             "%1 (built against %2)",
                             Quotient::versionString(),
                             QStringLiteral(Quotient_VERSION_STRING)),
#else
                       QStringLiteral(QUOTIENT_VERSION),
#endif
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
    UrlHelper urlHelper;

#ifdef HAVE_COLORSCHEME
    ColorSchemer colorScheme;
    qmlRegisterSingletonInstance<ColorSchemer>("org.kde.neochat", 1, 0, "ColorSchemer", &colorScheme);
    if (!config->colorScheme().isEmpty()) {
        colorScheme.apply(config->colorScheme());
    }
#endif

    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "Controller", &Controller::instance());
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "NotificationsManager", &NotificationsManager::instance());
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "Clipboard", &clipboard);
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "Config", config);
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "RoomManager", &RoomManager::instance());
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "FileType", &fileTypeSingleton);
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "LoginHelper", login);
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "UrlHelper", &urlHelper);
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "EmojiModel", &EmojiModel::instance());
#ifdef QUOTIENT_07
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "AccountRegistry", &Quotient::Accounts);
#else
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "AccountRegistry", &Quotient::AccountRegistry::instance());
#endif
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "SpaceHierarchyCache", &SpaceHierarchyCache::instance());
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "CustomEmojiModel", &CustomEmojiModel::instance());
    qmlRegisterType<ActionsHandler>("org.kde.neochat", 1, 0, "ActionsHandler");
    qmlRegisterType<ChatDocumentHandler>("org.kde.neochat", 1, 0, "ChatDocumentHandler");
    qmlRegisterType<RoomListModel>("org.kde.neochat", 1, 0, "RoomListModel");
    qmlRegisterType<KWebShortcutModel>("org.kde.neochat", 1, 0, "WebShortcutModel");
    qmlRegisterType<UserListModel>("org.kde.neochat", 1, 0, "UserListModel");
    qmlRegisterType<MessageEventModel>("org.kde.neochat", 1, 0, "MessageEventModel");
    qmlRegisterType<CollapseStateProxyModel>("org.kde.neochat", 1, 0, "CollapseStateProxyModel");
    qmlRegisterType<MessageFilterModel>("org.kde.neochat", 1, 0, "MessageFilterModel");
    qmlRegisterType<UserFilterModel>("org.kde.neochat", 1, 0, "UserFilterModel");
    qmlRegisterType<PublicRoomListModel>("org.kde.neochat", 1, 0, "PublicRoomListModel");
    qmlRegisterType<UserDirectoryListModel>("org.kde.neochat", 1, 0, "UserDirectoryListModel");
    qmlRegisterType<ServerListModel>("org.kde.neochat", 1, 0, "ServerListModel");
    qmlRegisterType<SortFilterRoomListModel>("org.kde.neochat", 1, 0, "SortFilterRoomListModel");
    qmlRegisterType<SortFilterSpaceListModel>("org.kde.neochat", 1, 0, "SortFilterSpaceListModel");
    qmlRegisterType<DevicesModel>("org.kde.neochat", 1, 0, "DevicesModel");
    qmlRegisterType<LinkPreviewer>("org.kde.neochat", 1, 0, "LinkPreviewer");
    qmlRegisterType<CompletionModel>("org.kde.neochat", 1, 0, "CompletionModel");
    qmlRegisterType<StateModel>("org.kde.neochat", 1, 0, "StateModel");
    qmlRegisterType<SearchModel>("org.kde.neochat", 1, 0, "SearchModel");
#ifdef QUOTIENT_07
    qmlRegisterType<PollHandler>("org.kde.neochat", 1, 0, "PollHandler");
#endif
    qmlRegisterType<KeywordNotificationRuleModel>("org.kde.neochat", 1, 0, "KeywordNotificationRuleModel");
    qmlRegisterUncreatableType<RoomMessageEvent>("org.kde.neochat", 1, 0, "RoomMessageEvent", "ENUM");
    qmlRegisterUncreatableType<PushNotificationState>("org.kde.neochat", 1, 0, "PushNotificationState", "ENUM");
    qmlRegisterUncreatableType<PushNotificationAction>("org.kde.neochat", 1, 0, "PushNotificationAction", "ENUM");
    qmlRegisterUncreatableType<NeoChatRoomType>("org.kde.neochat", 1, 0, "NeoChatRoomType", "ENUM");
    qmlRegisterUncreatableType<UserType>("org.kde.neochat", 1, 0, "UserType", "ENUM");
    qmlRegisterUncreatableType<NeoChatUser>("org.kde.neochat", 1, 0, "NeoChatUser", {});

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
#ifdef QUOTIENT_07
#ifdef Quotient_E2EE_ENABLED
    qRegisterMetaType<KeyVerificationSession *>("KeyVerificationSession*");
    qmlRegisterUncreatableType<KeyVerificationSession>("org.kde.neochat", 1, 0, "KeyVerificationSession", {});
    qRegisterMetaType<QVector<EmojiEntry>>("QVector<EmojiEntry>");
#endif
#endif
    qmlRegisterSingletonType("org.kde.neochat", 1, 0, "About", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
        return engine->toScriptValue(KAboutData::applicationData());
    });

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qRegisterMetaTypeStreamOperators<Emoji>();
#endif

    QQmlApplicationEngine engine;

#ifdef HAVE_KDBUSADDONS
    KDBusService service(KDBusService::Unique);
    service.connect(&service,
                    &KDBusService::activateRequested,
                    &RoomManager::instance(),
                    [&engine](const QStringList &arguments, const QString &workingDirectory) {
                        Q_UNUSED(workingDirectory);

                        QWindow *window = windowFromEngine(&engine);
                        KWindowSystem::updateStartupId(window);

                        WindowController::instance().showAndRaiseWindow(QString());

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

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);
    engine.setNetworkAccessManagerFactory(new NetworkAccessManagerFactory());

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Client for the matrix communication protocol"));
    parser.addPositionalArgument(QStringLiteral("urls"), i18n("Supports matrix: url scheme"));

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    engine.addImageProvider(QLatin1String("mxc"), new MatrixImageProvider);
    engine.addImageProvider(QLatin1String("blurhash"), new BlurhashImageProvider);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (parser.positionalArguments().length() > 0) {
        RoomManager::instance().setUrlArgument(parser.positionalArguments()[0]);
    }

#ifdef HAVE_RUNNER
    Runner runner;
    QDBusConnection::sessionBus().registerObject("/RoomRunner", &runner, QDBusConnection::ExportScriptableContents);
#endif

    QWindow *window = windowFromEngine(&engine);

    WindowController::instance().setWindow(window);
    WindowController::instance().restoreGeometry();

    return app.exec();
}
