#include "imageprovider.h"

#include <QtCore/QDebug>
#include <QtCore/QThread>

using QMatrixClient::BaseJob;

ThumbnailResponse::ThumbnailResponse(QMatrixClient::Connection* c,
    QString mediaId, const QSize& requestedSize)
  : c(c),
    mediaId(std::move(mediaId)),
    requestedSize(requestedSize),
    errorStr("Image request hasn't started") {
  if (requestedSize.isEmpty()) {
    errorStr.clear();
    emit finished();
    return;
  }
  if (mediaId.count('/') != 1) {
    errorStr =
        tr("Media id '%1' doesn't follow server/mediaId pattern")
            .arg(mediaId);
    emit finished();
    return;
  }
  // Execute a request on the main thread asynchronously
  moveToThread(c->thread());
  QMetaObject::invokeMethod(this, &ThumbnailResponse::startRequest,
                            Qt::QueuedConnection);
}

void ThumbnailResponse::startRequest() {
  // Runs in the main thread, not QML thread
  Q_ASSERT(QThread::currentThread() == c->thread());
  job = c->getThumbnail(mediaId, requestedSize);
  // Connect to any possible outcome including abandonment
  // to make sure the QML thread is not left stuck forever.
  connect(job, &BaseJob::finished, this, &ThumbnailResponse::prepareResult);
}

void ThumbnailResponse::prepareResult() {
  Q_ASSERT(QThread::currentThread() == job->thread());
  Q_ASSERT(job->error() != BaseJob::Pending);  
  {
    QWriteLocker _(&lock);
    if (job->error() == BaseJob::Success) {
      image = job->thumbnail();
      errorStr.clear();
    } else if (job->error() == BaseJob::Abandoned) {
      errorStr = tr("Image request has been cancelled");
      qDebug() << "ThumbnailResponse: cancelled for" << mediaId;  
    } else {
      errorStr = job->errorString();
      qWarning() << "ThumbnailResponse: no valid image for" << mediaId << "-"
                 << errorStr;
    }
    job = nullptr;
  }
  emit finished();
}

void ThumbnailResponse::doCancel() {
  // Runs in the main thread, not QML thread
  Q_ASSERT(QThread::currentThread() == job->thread());
  if (job)
    job->abandon();  
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
  QMetaObject::invokeMethod(this, &ThumbnailResponse::doCancel,
                            Qt::QueuedConnection);
}

QQuickImageResponse* ImageProvider::requestImageResponse(
    const QString& id, const QSize& requestedSize) {
  qDebug() << "ImageProvider: requesting " << id;
  return new ThumbnailResponse(m_connection.load(), id, requestedSize);
}
