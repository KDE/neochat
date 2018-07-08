#ifndef IMAGEPROVIDERCONNECTION_H
#define IMAGEPROVIDERCONNECTION_H

#include <QObject>

#include "connection.h"

class ImageProviderConnection : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QMatrixClient::Connection* connection READ getConnection WRITE setConnection NOTIFY connectionChanged)

    public:
        explicit ImageProviderConnection(QObject* parent = nullptr);
        ~ImageProviderConnection();

        QMatrixClient::Connection* getConnection() { return m_connection; }
        void setConnection(QMatrixClient::Connection* connection) {
            emit connectionChanged();
            m_connection = connection;
        }
    private:
        QMatrixClient::Connection* m_connection;
    signals:
        void connectionChanged();
};

#endif // IMAGEPROVIDERCONNECTION_H
