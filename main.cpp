#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "connection.h"
#include "room.h"
#include "user.h"
#include "jobs/syncjob.h"
#include "settings.h"
using namespace QMatrixClient;

// https://forum.qt.io/topic/57809
Q_DECLARE_METATYPE(SyncJob*)
Q_DECLARE_METATYPE(Room*)

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    qmlRegisterType<SyncJob>();
    qRegisterMetaType<SyncJob*> ("SyncJob*");
    qmlRegisterType<Room>();
    qRegisterMetaType<Room*>("Room*");
    qmlRegisterType<User>();
    qRegisterMetaType<User*>("User*");

    qmlRegisterType<Connection>("Matrique", 0, 1, "Connection");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
