/**
 * SPDX-FileCopyrightText: Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2019 Kitsune Ral <kitsune-ral@users.sf.net>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-only
 */
#ifndef MatrixImageProvider_H
#define MatrixImageProvider_H
#pragma once

#include <QQuickAsyncImageProvider>

#include <connection.h>
#include <jobs/mediathumbnailjob.h>

#include <QAtomicPointer>
#include <QReadWriteLock>

namespace Quotient
{
class Connection;
}

class ThumbnailResponse : public QQuickImageResponse
{
    Q_OBJECT
public:
    ThumbnailResponse(QString mediaId, QSize requestedSize);
    ~ThumbnailResponse() override = default;

private Q_SLOTS:
    void startRequest();
    void prepareResult();
    void doCancel();

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
    void cancel() override;
};

class MatrixImageProvider : public QQuickAsyncImageProvider
{
public:
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
};

#endif // MatrixImageProvider_H
