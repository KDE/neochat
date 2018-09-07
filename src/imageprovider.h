#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QObject>
#include <QtCore/QReadWriteLock>
#include <QtQuick/QQuickImageProvider>

#include "connection.h"

class ImageProvider : public QObject, public QQuickImageProvider {
  Q_OBJECT
  Q_PROPERTY(QMatrixClient::Connection* connection READ connection WRITE
                 setConnection NOTIFY connectionChanged)
 public:
  explicit ImageProvider(QObject* parent = nullptr);

  QImage requestImage(const QString& id, QSize* pSize,
                      const QSize& requestedSize) override;

  void initializeEngine(QQmlEngine* engine, const char* uri);

  QMatrixClient::Connection* connection() { return m_connection; }
  void setConnection(QMatrixClient::Connection* newConnection) {
    if (m_connection != newConnection) {
      m_connection = newConnection;
      emit connectionChanged();
    }
  }

 signals:
  void connectionChanged();

 private:
  QReadWriteLock m_lock;
  QMatrixClient::Connection* m_connection;
};

#endif  // IMAGEPROVIDER_H
