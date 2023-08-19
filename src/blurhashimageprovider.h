// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QQuickAsyncImageProvider>
#include <QThreadPool>

class AsyncImageResponse : public QQuickImageResponse
{
public:
    AsyncImageResponse(const QString &id, const QSize &requestedSize, QThreadPool *pool);
    void handleDone(QImage image);
    QQuickTextureFactory *textureFactory() const override;
    QImage m_image;
};

class BlurhashImageProvider : public QQuickAsyncImageProvider
{
public:
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;

private:
    QThreadPool pool;
};
