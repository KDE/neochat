#include <QApplication>
#include <QNetworkProxy>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "controller.h"
#include "imageprovider.h"
#include "messageeventmodel.h"
#include "room.h"
#include "roomlistmodel.h"

using namespace QMatrixClient;

int main(int argc, char *argv[]) {
#if defined(Q_OS_WIN)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  QApplication app(argc, argv);

  //    Enable this if you need proxy.
  //    QNetworkProxy proxy;
  //    proxy.setType(QNetworkProxy::HttpProxy);
  //    proxy.setHostName("localhost");
  //    proxy.setPort(1082);
  //    QNetworkProxy::setApplicationProxy(proxy);

  qRegisterMetaType<Room *>("Room*");
  qRegisterMetaType<User *>("User*");

  qmlRegisterType<Controller>("Matrique", 0, 1, "Controller");
  qmlRegisterType<RoomListModel>("Matrique", 0, 1, "RoomListModel");
  qmlRegisterType<MessageEventModel>("Matrique", 0, 1, "MessageEventModel");

  QQmlApplicationEngine engine;

  ImageProvider *m_provider = new ImageProvider();

  engine.rootContext()->setContextProperty("imageProvider",
                                           m_provider->getConnection());

  engine.addImageProvider(QLatin1String("mxc"), m_provider);

  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

  if (engine.rootObjects().isEmpty()) return -1;

  return app.exec();
}
