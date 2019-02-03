#include "imageprovider.h"

#include <connection.h>
#include <jobs/mediathumbnailjob.h>

#include <QtCore/QDebug>
#include <QtCore/QReadWriteLock>

using QMatrixClient::BaseJob;
using QMatrixClient::Connection;

ThumbnailResponse::ThumbnailResponse(Connection* c, QString mediaId,
                                     const QSize& requestedSize)
    : c(c),
      mediaId(std::move(mediaId)),
      requestedSize(requestedSize),
      errorStr("Image request hasn't started") {
  moveToThread(c->thread());
  if (requestedSize.isEmpty()) {
    errorStr.clear();
    emit finished();
    return;
  }
  // Execute a request on the main thread asynchronously
  QMetaObject::invokeMethod(this, &ThumbnailResponse::startRequest,
                            Qt::QueuedConnection);
}

void ThumbnailResponse::startRequest() {
  // Runs in the main thread, not QML thread
  if (mediaId.count('/') != 1) {
    errorStr =
        QStringLiteral("Media id '%1' doesn't follow server/mediaId pattern")
            .arg(mediaId);
    emit finished();
    return;
  }

  QWriteLocker _(&lock);
  job = c->getThumbnail(mediaId, requestedSize);
  // Connect to any possible outcome including abandonment
  // to make sure the QML thread is not left stuck forever.
  connect(job, &BaseJob::finished, this, &ThumbnailResponse::prepareResult);
}

void ThumbnailResponse::prepareResult() {
  {
    QWriteLocker _(&lock);
    Q_ASSERT(job->error() != BaseJob::Pending);

    if (job->error() == BaseJob::Success) {
      image = job->thumbnail();
      errorStr.clear();
    } else {
      errorStr = job->errorString();
      qWarning() << "ThumbnailResponse: no valid image for" << mediaId << "-"
                 << errorStr;
    }
    job = nullptr;
  }
  emit finished();
}

QQuickTextureFactory* ThumbnailResponse::textureFactory() const {
  QReadLocker _(&lock);
  return QQuickTextureFactory::textureFactoryForImage(image);
}

QString ThumbnailResponse::errorString() const {
  QReadLocker _(&lock);
  return errorStr;
}

void ThumbnailResponse::cancel() {
  QWriteLocker _(&lock);
  if (job) {
    job->abandon();
    job = nullptr;
  }
  errorStr = "Image request has been cancelled";
}

QQuickImageResponse* ImageProvider::requestImageResponse(
    const QString& id, const QSize& requestedSize) {
  qDebug() << "ImageProvider: requesting " << id;
  return new ThumbnailResponse(m_connection.load(), id, requestedSize);
}
