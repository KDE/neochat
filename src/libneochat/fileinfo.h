// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QMimeType>
#include <QSize>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

namespace Blocks
{
/**
 * @class FileInfo
 *
 * Base class to store the information about a file.
 *
 * Has the following parameters:
 *  - size - the file size in bytes
 *  - mimetype - the QMimeType for the file
 *
 * @sa QMimeType
 */
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

/**
 * @class ImageInfo
 *
 * Inherited class to store the information about an image.
 *
 * In addition to the parameters in FileInfo has the following:
 *  - pixelSize - the QSize of the image in pixels.
 *  - isAnimated - whether the image is animated.
 *  - isSticker - whether the image is a sticker.
 *
 * @sa FileInfo
 */
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

/**
 * @class VideoInfo
 *
 * Inherited class to store the information about a video.
 *
 * In addition to the parameters in FileInfo has the following:
 *  - pixelSize - the QSize of the video in pixels.
 *  - duration - the length of the video.
 *
 * @sa FileInfo
 */
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

/**
 * @class AudioInfo
 *
 * Inherited class to store the information about an audio file.
 *
 * In addition to the parameters in FileInfo has the following:
 *  - duration - the length of the video.
 *
 * @sa FileInfo
 */
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
