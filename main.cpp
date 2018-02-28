#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QNetworkProxy>

#include "matrix/controller.h"
#include "matrix/roomlistmodel.h"
using namespace QMatrixClient;

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

//    Enable this if you need proxy.
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName("localhost");
    proxy.setPort(1082);
    QNetworkProxy::setApplicationProxy(proxy);

    qmlRegisterType<Controller>("Matrique", 0, 1, "Controller");
    qmlRegisterType<RoomListModel>("Matrique", 0, 1, "RoomListModel");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
