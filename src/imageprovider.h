#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H
#pragma once

#include <jobs/mediathumbnailjob.h>
#include <QThreadPool>
#include <QtCore/QAtomicPointer>
#include <QtCore/QReadWriteLock>
#include <QtQuick/QQuickAsyncImageProvider>

namespace QMatrixClient {
class Connection;
}

class ThumbnailResponse : public QQuickImageResponse {
 public:
  ThumbnailResponse(QMatrixClient::Connection* c, QString mediaId,
                    const QSize& requestedSize);
  ~ThumbnailResponse() override = default;

  void startRequest();

 private:
  QMatrixClient::Connection* c;
  const QString mediaId;
  const QSize requestedSize;
  QMatrixClient::MediaThumbnailJob* job = nullptr;

  QImage image;
  QString errorStr;
  mutable QReadWriteLock lock;

  void prepareResult();
  QQuickTextureFactory* textureFactory() const override;
  QString errorString() const override;
  void cancel() override;
};

class ImageProvider : public QObject, public QQuickAsyncImageProvider {
  Q_OBJECT
  Q_PROPERTY(QMatrixClient::Connection* connection READ connection WRITE
                 setConnection NOTIFY connectionChanged)
 public:
  explicit ImageProvider() : QObject(), QQuickAsyncImageProvider() {}

  QQuickImageResponse* requestImageResponse(
      const QString& id, const QSize& requestedSize) override;

  QMatrixClient::Connection* connection() { return m_connection; }
  void setConnection(QMatrixClient::Connection* connection) {
    m_connection.store(connection);
  }

 signals:
  void connectionChanged();

 private:
  QAtomicPointer<QMatrixClient::Connection> m_connection;
};

#endif  // IMAGEPROVIDER_H
