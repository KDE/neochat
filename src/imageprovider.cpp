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
    : QQuickImageProvider(
          QQmlImageProviderBase::Image,
          QQmlImageProviderBase::ForceAsynchronousImageLoading) {
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
  qRegisterMetaType<MediaThumbnailJob*>();
#endif
  m_connection = new ImageProviderConnection();
}

QImage ImageProvider::requestImage(const QString& id, QSize* pSize,
                                   const QSize& requestedSize) {
  if (!id.startsWith("mxc://")) {
    qWarning() << "ImageProvider: won't fetch an invalid id:" << id
               << "doesn't follow server/mediaId pattern";
    return {};
  }

  QUrl mxcUri{id};

  QUrl tempfilePath = QUrl::fromLocalFile(
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/" +
      mxcUri.fileName() + "-" + QString::number(requestedSize.width()) + "x" +
      QString::number(requestedSize.height()) + ".png");

  QImage cachedImage;
  if (cachedImage.load(tempfilePath.toLocalFile())) {
    if (pSize != nullptr) *pSize = cachedImage.size();
    return cachedImage;
  }

  MediaThumbnailJob* job = nullptr;
  QReadLocker locker(&m_lock);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
  QMetaObject::invokeMethod(
      m_connection,
      [=] {
        return m_connection->getConnection()->getThumbnail(mxcUri,
                                                           requestedSize);
      },
      Qt::BlockingQueuedConnection, &job);
#else
  QMetaObject::invokeMethod(m_connection->getConnection(), "getThumbnail",
                            Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(MediaThumbnailJob*, job),
                            Q_ARG(QUrl, mxcUri), Q_ARG(QSize, requestedSize));
#endif
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

  if (pSize != nullptr) *pSize = result.size();

  result.save(tempfilePath.toLocalFile());

  return result;
}
