#include <QFontDatabase>
#include <QGuiApplication>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "accountlistmodel.h"
#include "controller.h"
#include "csapi/joining.h"
#include "csapi/leaving.h"
#include "emojimodel.h"
#include "imageclipboard.h"
#include "matriximageprovider.h"
#include "messageeventmodel.h"
#include "notifications/manager.h"
#include "publicroomlistmodel.h"
#include "room.h"
#include "roomlistmodel.h"
#include "spectralroom.h"
#include "spectraluser.h"
#include "trayicon.h"
#include "userlistmodel.h"

using namespace Quotient;

int main(int argc, char* argv[]) {
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QNetworkProxyFactory::setUseSystemConfiguration(true);

  QApplication app(argc, argv);

  app.setOrganizationName("ENCOM");
  app.setOrganizationDomain("encom.eu.org");
  app.setApplicationName("Spectral");
  app.setWindowIcon(QIcon(":/assets/img/icon.png"));

  qmlRegisterType<Controller>("Spectral", 0, 1, "Controller");
  qmlRegisterType<AccountListModel>("Spectral", 0, 1, "AccountListModel");
  qmlRegisterType<RoomListModel>("Spectral", 0, 1, "RoomListModel");
  qmlRegisterType<UserListModel>("Spectral", 0, 1, "UserListModel");
  qmlRegisterType<MessageEventModel>("Spectral", 0, 1, "MessageEventModel");
  qmlRegisterType<PublicRoomListModel>("Spectral", 0, 1, "PublicRoomListModel");
  qmlRegisterType<EmojiModel>("Spectral", 0, 1, "EmojiModel");
  qmlRegisterType<NotificationsManager>("Spectral", 0, 1,
                                        "NotificationsManager");
  qmlRegisterType<TrayIcon>("Spectral", 0, 1, "TrayIcon");
  qmlRegisterType<ImageClipboard>("Spectral", 0, 1, "ImageClipboard");
  qmlRegisterUncreatableType<RoomMessageEvent>("Spectral", 0, 1,
                                               "RoomMessageEvent", "ENUM");
  qmlRegisterUncreatableType<RoomType>("Spectral", 0, 1, "RoomType", "ENUM");

  qRegisterMetaType<User*>("User*");
  qRegisterMetaType<User*>("const User*");
  qRegisterMetaType<User*>("const Quotient::User*");
  qRegisterMetaType<Room*>("Room*");
  qRegisterMetaType<Connection*>("Connection*");
  qRegisterMetaType<MessageEventType>("MessageEventType");
  qRegisterMetaType<SpectralRoom*>("SpectralRoom*");
  qRegisterMetaType<SpectralUser*>("SpectralUser*");
  qRegisterMetaType<GetRoomEventsJob*>("GetRoomEventsJob*");

  qRegisterMetaTypeStreamOperators<Emoji>();

  QQmlApplicationEngine engine;

  engine.addImportPath("qrc:/imports");
  MatrixImageProvider* matrixImageProvider = new MatrixImageProvider();
  engine.rootContext()->setContextProperty("imageProvider",
                                           matrixImageProvider);
  engine.addImageProvider(QLatin1String("mxc"), matrixImageProvider);

  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
