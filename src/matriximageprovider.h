#ifndef MatrixImageProvider_H
#define MatrixImageProvider_H
#pragma once

#include <QtQuick/QQuickAsyncImageProvider>

#include <connection.h>
#include <jobs/mediathumbnailjob.h>

#include <QtCore/QAtomicPointer>
#include <QtCore/QReadWriteLock>

namespace Quotient {
class Connection;
}

class ThumbnailResponse : public QQuickImageResponse {
  Q_OBJECT
 public:
  ThumbnailResponse(Quotient::Connection* c,
                    QString mediaId,
                    const QSize& requestedSize);
  ~ThumbnailResponse() override = default;

 private slots:
  void startRequest();
  void prepareResult();
  void doCancel();

 private:
  Quotient::Connection* c;
  const QString mediaId;
  const QSize requestedSize;
  const QString localFile;
  Quotient::MediaThumbnailJob* job = nullptr;

  QImage image;
  QString errorStr;
  mutable QReadWriteLock lock;  // Guards ONLY these two members above

  QQuickTextureFactory* textureFactory() const override;
  QString errorString() const override;
  void cancel() override;
};

class MatrixImageProvider : public QObject, public QQuickAsyncImageProvider {
  Q_OBJECT
  Q_PROPERTY(Quotient::Connection* connection READ connection WRITE
                 setConnection NOTIFY connectionChanged)
 public:
  explicit MatrixImageProvider() = default;

  QQuickImageResponse* requestImageResponse(
      const QString& id,
      const QSize& requestedSize) override;

  Quotient::Connection* connection() { return m_connection; }
  void setConnection(Quotient::Connection* connection) {
    m_connection.store(connection);
    emit connectionChanged();
  }

 signals:
  void connectionChanged();

 private:
  QAtomicPointer<Quotient::Connection> m_connection;
};

#endif  // MatrixImageProvider_H
