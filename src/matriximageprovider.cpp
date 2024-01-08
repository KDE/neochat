// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2019 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-License-Identifier: GPL-3.0-only

#include "matriximageprovider.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QThread>

#include <KLocalizedString>

#include "neochatconnection.h"

#include <Quotient/connection.h>

using namespace Quotient;

ThumbnailResponse::ThumbnailResponse(QString id, QSize size, NeoChatConnection *connection)
    : mediaId(std::move(id))
    , requestedSize(size)
    , localFile(QStringLiteral("%1/image_provider/%2-%3x%4.png")
                    .arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation),
                         mediaId,
                         QString::number(requestedSize.width()),
                         QString::number(requestedSize.height())))
    , m_connection(connection)
    , errorStr("Image request hasn't started"_ls)
{
    if (requestedSize.isEmpty()) {
        requestedSize.setHeight(100);
        requestedSize.setWidth(100);
    }
    if (mediaId.count(QLatin1Char('/')) != 1) {
        if (mediaId.startsWith(QLatin1Char('/'))) {
            mediaId = mediaId.mid(1);
        } else {
            errorStr = i18n("Media id '%1' doesn't follow server/mediaId pattern", mediaId);
            Q_EMIT finished();
            return;
        }
    }

    mediaId = mediaId.split(QLatin1Char('?'))[0];

    QImage cachedImage;
    if (cachedImage.load(localFile)) {
        image = cachedImage;
        errorStr.clear();
        Q_EMIT finished();
        return;
    }

    if (!m_connection) {
        qWarning() << "Current connection is null";
        return;
    }

    // Execute a request on the main thread asynchronously
    moveToThread(m_connection->thread());
    QMetaObject::invokeMethod(this, &ThumbnailResponse::startRequest, Qt::QueuedConnection);
}

void ThumbnailResponse::startRequest()
{
    if (!m_connection) {
        return;
    }
    // Runs in the main thread, not QML thread
    Q_ASSERT(QThread::currentThread() == m_connection->thread());
    job = m_connection->getThumbnail(mediaId, requestedSize);
    // Connect to any possible outcome including abandonment
    // to make sure the QML thread is not left stuck forever.
    connect(job, &BaseJob::finished, this, &ThumbnailResponse::prepareResult);
}

void ThumbnailResponse::prepareResult()
{
    Q_ASSERT(QThread::currentThread() == job->thread());
    Q_ASSERT(job->error() != BaseJob::Pending);
    {
        QWriteLocker _(&lock);
        if (job->error() == BaseJob::Success) {
            image = job->thumbnail();

            QString localPath = QFileInfo(localFile).absolutePath();
            QDir dir;
            if (!dir.exists(localPath)) {
                dir.mkpath(localPath);
            }

            image.save(localFile);

            errorStr.clear();
        } else if (job->error() == BaseJob::Abandoned) {
            errorStr = i18n("Image request has been cancelled");
            // qDebug() << "ThumbnailResponse: cancelled for" << mediaId;
        } else {
            errorStr = job->errorString();
            qWarning() << "ThumbnailResponse: no valid image for" << mediaId << "-" << errorStr;
        }
        job = nullptr;
    }
    Q_EMIT finished();
}

QQuickTextureFactory *ThumbnailResponse::textureFactory() const
{
    QReadLocker _(&lock);
    return QQuickTextureFactory::textureFactoryForImage(image);
}

QString ThumbnailResponse::errorString() const
{
    QReadLocker _(&lock);
    return errorStr;
}

QQuickImageResponse *MatrixImageProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    return new ThumbnailResponse(id, requestedSize, m_connection);
}

#include "moc_matriximageprovider.cpp"
