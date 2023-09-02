// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QSize>

/**
 * @class MediaSizeHelper
 *
 * A class to help calculate the current width of a media item within a chat delegate.
 *
 * The only realistic way to guarantee that a media item (e.g. an image or video)
 * is the correct size in QML is to calculate the size manually.
 *
 * The rules for this component work as follows:
 *  - The output will always try to keep the media size if no limits are breached.
 *  - If no media width is set, the current size will be a placeholder at a 16:9 ratio
 *    calcualated from either the configured max width or the contentMaxWidth, whichever
 *    is smaller (if the contentMaxWidth isn't set, the configured max width is used).
 *  - The aspect ratio of the media will always be maintained if set (otherwise 16:9).
 *  - The current size will never be larger than any of the limits in either direction.
 *  - If any limit is breached the image size will be reduced while maintaining aspect
 *    ration, i.e. no stretching or squashing. This can mean that the width or height
 *    is reduced even if that parameter doesn't breach the limit itself.
 */
class MediaSizeHelper : public QObject
{
    Q_OBJECT

    /**
     * @brief The maximum width (in px) the media can be.
     *
     * This is the upper limit placed upon the media by the delegate.
     */
    Q_PROPERTY(qreal contentMaxWidth READ contentMaxWidth WRITE setContentMaxWidth NOTIFY contentMaxWidthChanged)

    /**
     * @brief The maximum height (in px) the media can be.
     *
     * This is the upper limit placed upon the media by the delegate.
     */
    Q_PROPERTY(qreal contentMaxHeight READ contentMaxHeight WRITE setContentMaxHeight NOTIFY contentMaxHeightChanged)

    /**
     * @brief The base width (in px) of the media.
     */
    Q_PROPERTY(qreal mediaWidth READ mediaWidth WRITE setMediaWidth NOTIFY mediaWidthChanged)

    /**
     * @brief The base height (in px) of the media.
     */
    Q_PROPERTY(qreal mediaHeight READ mediaHeight WRITE setMediaHeight NOTIFY mediaHeightChanged)

    /**
     * @brief The size (in px) of the component based on the current input.
     *
     * Will always try to return a value even if some of the inputs are not set to
     * account for being called before the parameters are intialised. For any parameters
     * not set these will just be left out of the calcs.
     *
     * If no input values are provided a default placeholder value will be returned.
     */
    Q_PROPERTY(QSize currentSize READ currentSize NOTIFY currentSizeChanged)

public:
    explicit MediaSizeHelper(QObject *parent = nullptr);

    qreal contentMaxWidth() const;
    void setContentMaxWidth(qreal contentMaxWidth);

    qreal contentMaxHeight() const;
    void setContentMaxHeight(qreal contentMaxHeight);

    qreal mediaWidth() const;
    void setMediaWidth(qreal mediaWidth);

    qreal mediaHeight() const;
    void setMediaHeight(qreal mediaHeight);

    QSize currentSize() const;

Q_SIGNALS:
    void contentMaxWidthChanged();
    void contentMaxHeightChanged();
    void mediaWidthChanged();
    void mediaHeightChanged();
    void currentSizeChanged();

private:
    qreal m_contentMaxWidth = -1.0;
    qreal m_contentMaxHeight = -1.0;
    qreal m_mediaWidth = -1.0;
    qreal m_mediaHeight = -1.0;

    qreal resolvedMediaWidth() const;
    qreal resolvedMediaHeight() const;
    qreal aspectRatio() const;
    bool limitWidth() const;
    qreal widthLimit() const;
    qreal heightLimit() const;
};
