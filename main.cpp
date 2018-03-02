#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QNetworkProxy>
#include <QQmlContext>

#include "matrix/controller.h"
#include "matrix/roomlistmodel.h"
#include "matrix/imageprovider.h"

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

    Connection* m_connection = new Connection();
    ImageProvider* m_provider = new ImageProvider();
    m_provider->setConnection(m_connection);

    engine.rootContext()->setContextProperty("m_connection", m_connection);
    engine.addImageProvider(QLatin1String("mxc"), m_provider);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
