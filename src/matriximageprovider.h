// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2019 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QQuickAsyncImageProvider>

#include <Quotient/jobs/mediathumbnailjob.h>

#include <QReadWriteLock>

namespace Quotient
{
class Connection;
}

/**
 * @class ThumbnailResponse
 *
 * A QQuickImageResponse for an mxc image.
 *
 * @sa QQuickImageResponse
 */
class ThumbnailResponse : public QQuickImageResponse
{
    Q_OBJECT
public:
    explicit ThumbnailResponse(QString mediaId, QSize requestedSize);
    ~ThumbnailResponse() override = default;

private Q_SLOTS:
    void startRequest();
    void prepareResult();

private:
    const QString mediaId;
    QSize requestedSize;
    const QString localFile;
    Quotient::MediaThumbnailJob *job = nullptr;

    QImage image;
    QString errorStr;
    mutable QReadWriteLock lock; // Guards ONLY these two members above

    QQuickTextureFactory *textureFactory() const override;
    QString errorString() const override;
};

/**
 * @class MatrixImageProvider
 *
 * A QQuickAsyncImageProvider for mxc images.
 *
 * @sa QQuickAsyncImageProvider
 */
class MatrixImageProvider : public QQuickAsyncImageProvider
{
public:
    /**
     * @brief Return a job to provide the image with the given ID.
     *
     * @sa QQuickAsyncImageProvider::requestImageResponse
     */
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
};
