#include "imageprovider.h"

#include <QFile>
#include <QMetaObject>
#include <QStandardPaths>
#include <QtCore/QDebug>
#include <QtCore/QWaitCondition>

#include "jobs/mediathumbnailjob.h"

#include "connection.h"

using QMatrixClient::MediaThumbnailJob;

ImageProvider::ImageProvider(QObject* parent)
    : QObject(parent),
      QQuickImageProvider(
          QQmlImageProviderBase::Image,
          QQmlImageProviderBase::ForceAsynchronousImageLoading) {}

QImage ImageProvider::requestImage(const QString& id, QSize* pSize,
                                   const QSize& requestedSize) {
  if (!id.startsWith("mxc://")) {
    qWarning() << "ImageProvider: won't fetch an invalid id:" << id
               << "doesn't follow server/mediaId pattern";
    return {};
  }

  QUrl mxcUri{id};

  QImage result = image(mxcUri, requestedSize);
  if (result.isNull()) return {};
  if (!requestedSize.isEmpty() && result.size() != requestedSize) {
    QImage scaled = result.scaled(requestedSize, Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);
    if (pSize != nullptr) *pSize = scaled.size();
    return scaled;
  }
  if (pSize != nullptr) *pSize = result.size();
  return result;
}

QImage ImageProvider::image(const QUrl& mxc, const QSize& size) {
  QUrl tempfilePath = QUrl::fromLocalFile(
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/" +
      mxc.fileName() + ".png");
  QImage cachedImage;
  if (cachedImage.load(tempfilePath.toLocalFile())) {
    return cachedImage;
  }

  MediaThumbnailJob* job = nullptr;
  QReadLocker locker(&m_lock);

  QMetaObject::invokeMethod(
      m_connection, [=] { return m_connection->getThumbnail(mxc, size); },
      Qt::BlockingQueuedConnection, &job);

  if (!job) {
    qDebug() << "ImageProvider: failed to send a request";
    return {};
  }
  QImage result;
  {
    QWaitCondition condition;  // The most compact way to block on a signal
    job->connect(job, &MediaThumbnailJob::finished, job, [&] {
      result = job->thumbnail();
      condition.wakeAll();
    });
    condition.wait(&m_lock);
  }

  result.save(tempfilePath.toLocalFile());

  return result;
}
