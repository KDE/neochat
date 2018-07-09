#include <QGuiApplication>
#include <QNetworkProxy>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "matrix/controller.h"
#include "matrix/imageprovider.h"
#include "matrix/messageeventmodel.h"
#include "matrix/roomlistmodel.h"
#include "room.h"

using namespace QMatrixClient;

int main(int argc, char *argv[]) {
#if defined(Q_OS_WIN)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  QGuiApplication app(argc, argv);

  //    Enable this if you need proxy.
  //    QNetworkProxy proxy;
  //    proxy.setType(QNetworkProxy::HttpProxy);
  //    proxy.setHostName("localhost");
  //    proxy.setPort(1082);
  //    QNetworkProxy::setApplicationProxy(proxy);

  qmlRegisterType<Room>();
  qRegisterMetaType<Room *>("Room*");

  qmlRegisterType<Controller>("Matrique", 0, 1, "Controller");
  qmlRegisterType<RoomListModel>("Matrique", 0, 1, "RoomListModel");
  qmlRegisterType<MessageEventModel>("Matrique", 0, 1, "MessageEventModel");
  qRegisterMetaType<User *>("User*");

  QQmlApplicationEngine engine;

  ImageProvider *m_provider = new ImageProvider();

  engine.rootContext()->setContextProperty("imageProvider",
                                           m_provider->getConnection());

  engine.addImageProvider(QLatin1String("mxc"), m_provider);

  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

  if (engine.rootObjects().isEmpty()) return -1;

  return app.exec();
}
