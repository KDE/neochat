// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QMimeType>
#include <QSize>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

namespace Blocks
{
class FileInfo
{
    Q_GADGET
    QML_VALUE_TYPE(fileInfo)

    /**
     * @brief The size of the file in bytes.
     */
    Q_PROPERTY(qint64 size MEMBER size CONSTANT)

    /**
     * @brief The QMimeType of the file.
     *
     * @sa QMimeType
     */
    Q_PROPERTY(QMimeType mimeType MEMBER mimeType CONSTANT)

public:
    qint64 size;
    QMimeType mimeType;
};

class ImageInfo : public FileInfo
{
    Q_GADGET
    QML_VALUE_TYPE(imageInfo)

    /**
     * @brief The QSize of the image in pixels.
     */
    Q_PROPERTY(QSize pixelSize MEMBER pixelSize CONSTANT)

    /**
     * @brief Whether the image is animated.
     */
    Q_PROPERTY(bool isAnimated MEMBER isAnimated CONSTANT)

    /**
     * @brief Whether the image is a sticker.
     */
    Q_PROPERTY(bool isSticker MEMBER isSticker CONSTANT)

public:
    QSize pixelSize;
    bool isAnimated = false;
    bool isSticker = false;
};

using ThumbnailInfo = ImageInfo;

class VideoInfo : public FileInfo
{
    Q_GADGET
    QML_VALUE_TYPE(videoInfo)

    /**
     * @brief The QSize of the video in pixels.
     */
    Q_PROPERTY(QSize pixelSize MEMBER pixelSize CONSTANT)

    /**
     * @brief The duration of the video file.
     */
    Q_PROPERTY(qint64 duration MEMBER duration CONSTANT)

public:
    QSize pixelSize;
    qint64 duration;
};

class AudioInfo : public FileInfo
{
    Q_GADGET
    QML_VALUE_TYPE(audioInfo)

    /**
     * @brief The duration of the audio file.
     */
    Q_PROPERTY(qint64 duration MEMBER duration CONSTANT)

public:
    qint64 duration;
};
}
