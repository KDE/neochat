#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QtQuick/QQuickImageProvider>
#include <QtCore/QReadWriteLock>
#include <QObject>

#include "libqmatrixclient/connection.h"

class ImageProviderConnection: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QMatrixClient::Connection* connection READ getConnection WRITE setConnection NOTIFY connectionChanged)

    public:
        explicit ImageProviderConnection(QObject* parent = nullptr);
        ~ImageProviderConnection();

        QMatrixClient::Connection* getConnection() { return m_connection; }
        Q_INVOKABLE void setConnection(QMatrixClient::Connection* connection) {
            qDebug() << "Connection changed.";
            emit connectionChanged();
            m_connection = connection;
        }
    private:
        QMatrixClient::Connection* m_connection;
    signals:
        void connectionChanged();
};

class ImageProvider: public QQuickImageProvider
{
    public:
        explicit ImageProvider(QObject* parent = nullptr);

        QImage requestImage(const QString& id, QSize* pSize,
                              const QSize& requestedSize) override;

        void initializeEngine(QQmlEngine *engine, const char *uri);

        ImageProviderConnection* getConnection() { return m_connection; }

    private:
        QReadWriteLock m_lock;
        ImageProviderConnection* m_connection;
};

#endif // IMAGEPROVIDER_H
