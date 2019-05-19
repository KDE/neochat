#include <QFontDatabase>
#include <QGuiApplication>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "accountlistmodel.h"
#include "controller.h"
#include "emojimodel.h"
#include "imageclipboard.h"
#include "matriximageprovider.h"
#include "messageeventmodel.h"
#include "notifications/manager.h"
#include "room.h"
#include "roomlistmodel.h"
#include "spectralroom.h"
#include "spectraluser.h"
#include "userlistmodel.h"

#include "csapi/joining.h"
#include "csapi/leaving.h"

#include "qqmlsortfilterproxymodel.h"

using namespace QMatrixClient;

int main(int argc, char* argv[]) {
#if defined(Q_OS_LINUX) || defined(Q_OS_WIN) || defined(Q_OS_FREEBSD)
  if (qgetenv("QT_SCALE_FACTOR").size() == 0) {
    QSettings settings("ENCOM", "Spectral");
    float factor = settings.value("Interface/dpi", 100).toFloat() / 100;

    qDebug() << "DPI:" << factor;

    if (factor != -1)
      qputenv("QT_SCALE_FACTOR", QString::number(factor).toUtf8());
  }
#endif

  QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

  QNetworkProxyFactory::setUseSystemConfiguration(true);

  QApplication app(argc, argv);

  app.setOrganizationName("ENCOM");
  app.setOrganizationDomain("encom.eu.org");
  app.setApplicationName("Spectral");
  app.setWindowIcon(QIcon(":/assets/img/icon.png"));

  qmlRegisterType<qqsfpm::QQmlSortFilterProxyModel>("SortFilterProxyModel", 0,
                                                    2, "SortFilterProxyModel");
  qmlRegisterType<Controller>("Spectral", 0, 1, "Controller");
  qmlRegisterType<AccountListModel>("Spectral", 0, 1, "AccountListModel");
  qmlRegisterType<RoomListModel>("Spectral", 0, 1, "RoomListModel");
  qmlRegisterType<UserListModel>("Spectral", 0, 1, "UserListModel");
  qmlRegisterType<MessageEventModel>("Spectral", 0, 1, "MessageEventModel");
  qmlRegisterType<EmojiModel>("Spectral", 0, 1, "EmojiModel");
  qmlRegisterType<NotificationsManager>("Spectral", 0, 1,
                                        "NotificationsManager");
  qmlRegisterType<ImageClipboard>("Spectral", 0, 1, "ImageClipboard");
  qmlRegisterUncreatableType<RoomMessageEvent>("Spectral", 0, 1,
                                               "RoomMessageEvent", "ENUM");
  qmlRegisterUncreatableType<RoomType>("Spectral", 0, 1, "RoomType", "ENUM");

  qRegisterMetaType<User*>("User*");
  qRegisterMetaType<User*>("const User*");
  qRegisterMetaType<Room*>("Room*");
  qRegisterMetaType<Connection*>("Connection*");
  qRegisterMetaType<MessageEventType>("MessageEventType");
  qRegisterMetaType<SpectralRoom*>("SpectralRoom*");
  qRegisterMetaType<SpectralUser*>("SpectralUser*");

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
