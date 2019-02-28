#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H
#pragma once

#include <QtQuick/QQuickAsyncImageProvider>
#include <QtCore/QAtomicPointer>

namespace QMatrixClient {
class Connection;
}

class ThumbnailResponse : public QQuickImageResponse {
  Q_OBJECT
 public:
  ThumbnailResponse(QMatrixClient::Connection* c, QString mediaId,
                    const QSize& requestedSize);
  ~ThumbnailResponse() override = default;

private slots:
  void startRequest();
  void prepareResult();
  void doCancel();

 private:
  QMatrixClient::Connection* c;
  const QString mediaId;
  const QSize requestedSize;
  QMatrixClient::MediaThumbnailJob* job = nullptr;

  QImage image;
  QString errorStr;
  mutable QReadWriteLock lock; // Guards ONLY these two members above

  QQuickTextureFactory* textureFactory() const override;
  QString errorString() const override;
  void cancel() override;
};

class ImageProvider : public QObject, public QQuickAsyncImageProvider {
  Q_OBJECT
  Q_PROPERTY(QMatrixClient::Connection* connection READ connection WRITE
                 setConnection NOTIFY connectionChanged)
 public:
  explicit ImageProvider() = default;

  QQuickImageResponse* requestImageResponse(
      const QString& id, const QSize& requestedSize) override;

  QMatrixClient::Connection* connection() { return m_connection; }
  void setConnection(QMatrixClient::Connection* connection) {
    m_connection.store(connection);
    emit connectionChanged();
  }

 signals:
  void connectionChanged();

 private:
  QAtomicPointer<QMatrixClient::Connection> m_connection;
};

#endif  // IMAGEPROVIDER_H
