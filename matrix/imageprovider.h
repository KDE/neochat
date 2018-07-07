#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QtQuick/QQuickImageProvider>
#include <QtCore/QReadWriteLock>
#include <QObject>

#include "libqmatrixclient/connection.h"
#include "imageproviderconnection.h"

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
