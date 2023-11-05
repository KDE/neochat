// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2019 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QQuickAsyncImageProvider>

#include <Quotient/jobs/mediathumbnailjob.h>

#include <QReadWriteLock>

class NeoChatConnection;

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
    explicit ThumbnailResponse(QString mediaId, QSize requestedSize, NeoChatConnection *m_connection);
    ~ThumbnailResponse() override = default;

private Q_SLOTS:
    void startRequest();
    void prepareResult();

private:
    QString mediaId;
    QSize requestedSize;
    const QString localFile;
    Quotient::MediaThumbnailJob *job = nullptr;
    NeoChatConnection *m_connection;

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
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(NeoChatConnection *connection MEMBER m_connection)
public:
    static MatrixImageProvider *create(QQmlEngine *engine, QJSEngine *)
    {
        static MatrixImageProvider instance;
        engine->setObjectOwnership(&instance, QQmlEngine::CppOwnership);
        return &instance;
    }

    /**
     * @brief Return a job to provide the image with the given ID.
     *
     * @sa QQuickAsyncImageProvider::requestImageResponse
     */
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;

private:
    NeoChatConnection *m_connection = nullptr;
    MatrixImageProvider() = default;
};
