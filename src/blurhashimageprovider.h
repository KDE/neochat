// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QQuickAsyncImageProvider>
#include <QThreadPool>

class AsyncImageResponse final : public QQuickImageResponse
{
public:
    AsyncImageResponse(const QString &id, const QSize &requestedSize, QThreadPool *pool);
    void handleDone(QImage image);
    QQuickTextureFactory *textureFactory() const override;
    QImage m_image;
};

class BlurHashImageProvider : public QQuickAsyncImageProvider
{
public:
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;

private:
    QThreadPool pool;
};
