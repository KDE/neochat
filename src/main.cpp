/**
 * SPDX-FileCopyrightText: Black Hat <bhat@encom.eu.org>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-only
 */
#include <QCommandLineParser>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>

#include "neochat-version.h"

#include "accountlistmodel.h"
#include "controller.h"
#include "csapi/joining.h"
#include "csapi/leaving.h"
#include "emojimodel.h"
#include "clipboard.h"
#include "matriximageprovider.h"
#include "messageeventmodel.h"
#include "notificationsmanager.h"
#include "publicroomlistmodel.h"
#include "room.h"
#include "roomlistmodel.h"
#include "sortfilterroomlistmodel.h"
#include "neochatroom.h"
#include "neochatuser.h"
#include "trayicon.h"
#include "userdirectorylistmodel.h"
#include "userlistmodel.h"

using namespace Quotient;

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif
int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QNetworkProxyFactory::setUseSystemConfiguration(true);

#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Material"));
#else
    QApplication app(argc, argv);
#endif

    app.setOrganizationName("KDE");
    app.setWindowIcon(QIcon(":/assets/img/icon.png"));

    Clipboard clipboard;

    qmlRegisterSingletonInstance("org.kde.neochat", 0, 1, "Controller", &Controller::instance());
    qmlRegisterSingletonInstance("org.kde.neochat", 1, 0, "Clipboard", &clipboard);
    qmlRegisterType<AccountListModel>("org.kde.neochat", 0, 1, "AccountListModel");
    qmlRegisterType<RoomListModel>("org.kde.neochat", 0, 1, "RoomListModel");
    qmlRegisterType<UserListModel>("org.kde.neochat", 0, 1, "UserListModel");
    qmlRegisterType<MessageEventModel>("org.kde.neochat", 0, 1, "MessageEventModel");
    qmlRegisterType<PublicRoomListModel>("org.kde.neochat", 0, 1, "PublicRoomListModel");
    qmlRegisterType<UserDirectoryListModel>("org.kde.neochat", 0, 1, "UserDirectoryListModel");
    qmlRegisterType<EmojiModel>("org.kde.neochat", 0, 1, "EmojiModel");
    qmlRegisterType<NotificationsManager>("org.kde.neochat", 0, 1, "NotificationsManager");
    qmlRegisterType<TrayIcon>("org.kde.neochat", 0, 1, "TrayIcon");
    qmlRegisterType<SortFilterRoomListModel>("org.kde.neochat", 0, 1, "SortFilterRoomListModel");
    qmlRegisterUncreatableType<RoomMessageEvent>("org.kde.neochat", 0, 1, "RoomMessageEvent", "ENUM");
    qmlRegisterUncreatableType<RoomType>("org.kde.neochat", 0, 1, "RoomType", "ENUM");
    qmlRegisterUncreatableType<UserType>("org.kde.neochat", 0, 1, "UserType", "ENUM");

    qRegisterMetaType<User *>("User*");
    qRegisterMetaType<User *>("const User*");
    qRegisterMetaType<User *>("const Quotient::User*");
    qRegisterMetaType<Room *>("Room*");
    qRegisterMetaType<Connection *>("Connection*");
    qRegisterMetaType<MessageEventType>("MessageEventType");
    qRegisterMetaType<NeoChatRoom *>("NeoChatRoom*");
    qRegisterMetaType<NeoChatUser *>("NeoChatUser*");
    qRegisterMetaType<GetRoomEventsJob *>("GetRoomEventsJob*");

    qRegisterMetaTypeStreamOperators<Emoji>();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    KLocalizedString::setApplicationDomain("neochat");

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Client for the matrix communication protocol"));

    KAboutData about(QStringLiteral("neochat"), i18n("Neochat"), QStringLiteral(NEOCHAT_VERSION_STRING), i18n("Matrix client"), KAboutLicense::GPL_V3, i18n("© 2018-2020 Black Hat, 2020 KDE Community"));
    about.addAuthor(i18n("Black Hat"), QString(), QStringLiteral("bhat@encom.eu.org"));
    about.addAuthor(i18n("Carl Schwan"), QString(), QStringLiteral("carl@carlschwan.eu"));
    about.addAuthor(i18n("Tobias Fella"), QString(), QStringLiteral("fella@posteo.de"));

    KAboutData::setApplicationData(about);

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    Controller::instance().setAboutData(about);

    engine.addImportPath("qrc:/imports");
    engine.addImageProvider(QLatin1String("mxc"), new MatrixImageProvider);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
