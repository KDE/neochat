// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include <QCommandLineParser>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QDebug>
#include <QIcon>

#ifdef Q_OS_ANDROID
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#include <KAboutData>
#ifdef HAVE_KDBUSADDONS
#include <KDBusService>
#endif
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KWindowConfig>

#include "neochat-version.h"

#include "accountlistmodel.h"
#include "actionshandler.h"
#include "chatboxhelper.h"
#include "chatdocumenthandler.h"
#include "clipboard.h"
#include "commandmodel.h"
#include "controller.h"
#include "csapi/joining.h"
#include "csapi/leaving.h"
#include "devicesmodel.h"
#include "emojimodel.h"
#include "filetypesingleton.h"
#include "login.h"
#include "matriximageprovider.h"
#include "messageeventmodel.h"
#include "messagefiltermodel.h"
#include "neochatconfig.h"
#include "neochatroom.h"
#include "neochatuser.h"
#include "notificationsmanager.h"
#include "publicroomlistmodel.h"
#include <room.h>
#include "roomlistmodel.h"
#include "roommanager.h"
#include "sortfilterroomlistmodel.h"
#include "userdirectorylistmodel.h"
#include "userlistmodel.h"
#include "webshortcutmodel.h"
#include "spellcheckhighlighter.h"
#include "customemojimodel.h"
#include "blurhashimageprovider.h"
#ifdef HAVE_COLORSCHEME
#include "colorschemer.h"
#endif

using namespace Quotient;

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif
int main(int argc, char *argv[])
{
    QIcon::setFallbackThemeName("breeze");
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QNetworkProxyFactory::setUseSystemConfiguration(true);

#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Material"));
#else
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
                     i18n("© 2018-2020 Black Hat, 2020 KDE Community"));
    about.addAuthor(i18n("Black Hat"), QString(), QStringLiteral("bhat@encom.eu.org"));
    about.addAuthor(i18n("Carl Schwan"), QString(), QStringLiteral("carl@carlschwan.eu"));
    about.addAuthor(i18n("Tobias Fella"), QString(), QStringLiteral("fella@posteo.de"));
    about.setOrganizationDomain("kde.org");

    about.addComponent(QStringLiteral("libQuotient"), i18n("A Qt5 library to write cross-platform clients for Matrix"), QString(), QStringLiteral("https://github.com/quotient-im/libquotient"), KAboutLicense::LGPL_V2_1);

    KAboutData::setApplicationData(about);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.neochat")));

#ifdef HAVE_KDBUSADDONS
    KDBusService service(KDBusService::Unique);
    service.connect(&service,
                    &KDBusService::activateRequested,
                    &RoomManager::instance(),
                    [](const QStringList &arguments, const QString &workingDirectory) {
                        Q_UNUSED(workingDirectory);
                        if(arguments.isEmpty()) {
                            return;
                        }
                        auto args = arguments;
                        args.removeFirst();
                        for (const auto &arg : args) {
                            RoomManager::instance().openResource(arg);
                        }
                    });
#endif

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
    qmlRegisterType<AccountListModel>("org.kde.neochat", 1, 0, "AccountListModel");
    qmlRegisterType<ActionsHandler>("org.kde.neochat", 1, 0, "ActionsHandler");
    qmlRegisterType<ChatDocumentHandler>("org.kde.neochat", 1, 0, "ChatDocumentHandler");
    qmlRegisterType<SpellcheckHighlighter>("org.kde.neochat", 1, 0, "SpellcheckHighlighter");
    qmlRegisterType<RoomListModel>("org.kde.neochat", 1, 0, "RoomListModel");
    qmlRegisterType<KWebShortcutModel>("org.kde.neochat", 1, 0, "WebShortcutModel");
    qmlRegisterType<UserListModel>("org.kde.neochat", 1, 0, "UserListModel");
    qmlRegisterType<CustomEmojiModel>("org.kde.neochat", 1, 0, "CustomEmojiModel");
    qmlRegisterType<MessageEventModel>("org.kde.neochat", 1, 0, "MessageEventModel");
    qmlRegisterType<MessageFilterModel>("org.kde.neochat", 1, 0, "MessageFilterModel");
    qmlRegisterType<PublicRoomListModel>("org.kde.neochat", 1, 0, "PublicRoomListModel");
    qmlRegisterType<UserDirectoryListModel>("org.kde.neochat", 1, 0, "UserDirectoryListModel");
    qmlRegisterType<SortFilterRoomListModel>("org.kde.neochat", 1, 0, "SortFilterRoomListModel");
    qmlRegisterType<DevicesModel>("org.kde.neochat", 1, 0, "DevicesModel");
    qmlRegisterUncreatableType<RoomMessageEvent>("org.kde.neochat", 1, 0, "RoomMessageEvent", "ENUM");
    qmlRegisterUncreatableType<RoomType>("org.kde.neochat", 1, 0, "RoomType", "ENUM");
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

    qRegisterMetaTypeStreamOperators<Emoji>();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    KLocalizedString::setApplicationDomain("neochat");
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);

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

#ifdef HAVE_KDBUSADDONS
    QObject::connect(&service, &KDBusService::activateRequested, &engine, [&engine](const QStringList & /*arguments*/, const QString & /*workingDirectory*/) {
        const auto rootObjects = engine.rootObjects();
        for (auto obj : rootObjects) {
            auto view = qobject_cast<QQuickWindow *>(obj);
            if (view) {
                view->show();
                view->raise();
                return;
            }
        }
    });
    const auto rootObjects = engine.rootObjects();
    for (auto obj : rootObjects) {
        auto view = qobject_cast<QQuickWindow *>(obj);
        if (view) {
            KConfig dataResource("data", KConfig::SimpleConfig, QStandardPaths::AppDataLocation);
            KConfigGroup windowGroup(&dataResource, "Window");
            KWindowConfig::restoreWindowSize(view, windowGroup);
            KWindowConfig::restoreWindowPosition(view, windowGroup);
            break;
        }
    }
#endif
    return app.exec();
}
