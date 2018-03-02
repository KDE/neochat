#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QtQuick/QQuickImageProvider>
#include <QtCore/QReadWriteLock>

namespace QMatrixClient {
    class Connection;
}

class ImageProvider: public QQuickImageProvider
{
    public:
        explicit ImageProvider(QObject *parent = nullptr);

        QImage requestImage(const QString& id, QSize* pSize,
                              const QSize& requestedSize) override;

        void setConnection(QMatrixClient::Connection* connection);

        void initializeEngine(QQmlEngine *engine, const char *uri);

    private:
        QMatrixClient::Connection* m_connection;
        QReadWriteLock m_lock;
};

#endif // IMAGEPROVIDER_H
