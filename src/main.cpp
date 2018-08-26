#include <QApplication>
#include <QNetworkProxy>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "controller.h"
#include "emojimodel.h"
#include "imageprovider.h"
#include "matriqueroom.h"
#include "messageeventmodel.h"
#include "room.h"
#include "roomlistmodel.h"
#include "userlistmodel.h"

#include "csapi/joining.h"
#include "csapi/leaving.h"

using namespace QMatrixClient;

int main(int argc, char *argv[]) {
#if defined(Q_OS_WIN)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  QApplication app(argc, argv);

  app.setOrganizationName("ENCOM");
  app.setOrganizationDomain("encom.eu.org");
  app.setApplicationName("Matrique");

  app.setQuitOnLastWindowClosed(false);

  qRegisterMetaType<MatriqueRoom *>("MatriqueRoom*");
  qRegisterMetaType<User *>("User*");
  qRegisterMetaType<MessageEventType>("MessageEventType");
  qRegisterMetaType<MatriqueRoom *>("MatriqueRoom");

  qmlRegisterType<Controller>("Matrique", 0, 1, "Controller");
  qmlRegisterType<RoomListModel>("Matrique", 0, 1, "RoomListModel");
  qmlRegisterType<UserListModel>("Matrique", 0, 1, "UserListModel");
  qmlRegisterType<MessageEventModel>("Matrique", 0, 1, "MessageEventModel");
  qmlRegisterType<EmojiModel>("Matrique", 0, 1, "EmojiModel");
  qmlRegisterUncreatableType<RoomMessageEvent>("Matrique", 0, 1,
                                               "RoomMessageEvent", "ENUM");
  qmlRegisterUncreatableType<RoomType>("Matrique", 0, 1, "RoomType", "ENUM");
  qmlRegisterSingletonType(QUrl("qrc:/qml/MatriqueSettings.qml"),
                           "Matrique.Settings", 0, 1, "MSettings");

  QQmlApplicationEngine engine;

  ImageProvider *m_provider = new ImageProvider();

  engine.rootContext()->setContextProperty("imageProvider",
                                           m_provider->getConnection());

  engine.addImageProvider(QLatin1String("mxc"), m_provider);

  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

  if (engine.rootObjects().isEmpty()) return -1;

  return app.exec();
}
