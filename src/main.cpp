#include <QApplication>
#include <QNetworkProxy>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "accountlistmodel.h"
#include "controller.h"
#include "emojimodel.h"
#include "imageitem.h"
#include "imageprovider.h"
#include "messageeventmodel.h"
#include "room.h"
#include "roomlistmodel.h"
#include "spectralroom.h"
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
  app.setApplicationName("Spectral");

  app.setQuitOnLastWindowClosed(false);

  qRegisterMetaType<SpectralRoom *>("SpectralRoom*");
  qRegisterMetaType<User *>("User*");
  qRegisterMetaType<MessageEventType>("MessageEventType");
  qRegisterMetaType<SpectralRoom *>("SpectralRoom");

  qmlRegisterType<ImageItem>("Spectral", 0, 1, "ImageItem");
  qmlRegisterType<Controller>("Spectral", 0, 1, "Controller");
  qmlRegisterType<AccountListModel>("Spectral", 0, 1, "AccountListModel");
  qmlRegisterType<RoomListModel>("Spectral", 0, 1, "RoomListModel");
  qmlRegisterType<UserListModel>("Spectral", 0, 1, "UserListModel");
  qmlRegisterType<MessageEventModel>("Spectral", 0, 1, "MessageEventModel");
  qmlRegisterType<EmojiModel>("Spectral", 0, 1, "EmojiModel");
  qmlRegisterUncreatableType<RoomMessageEvent>("Spectral", 0, 1,
                                               "RoomMessageEvent", "ENUM");
  qmlRegisterUncreatableType<RoomType>("Spectral", 0, 1, "RoomType", "ENUM");

  qmlRegisterSingletonType(QUrl("qrc:/imports/Spectral/Setting/Setting.qml"),
                           "Spectral.Setting", 0, 1, "MSettings");
  qmlRegisterSingletonType(
      QUrl("qrc:/imports/Spectral/Font/MaterialFont.qml"), "Spectral.Font",
      0, 1, "MaterialFont");
  qmlRegisterModule("qrc:/qml/component", 2, 0);

  QQmlApplicationEngine engine;

  engine.addImportPath("qrc:/imports");

  ImageProvider *m_provider = new ImageProvider();

  engine.rootContext()->setContextProperty("imageProvider", m_provider);

  engine.addImageProvider(QLatin1String("mxc"), m_provider);

  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

  if (engine.rootObjects().isEmpty()) return -1;

  return app.exec();
}
