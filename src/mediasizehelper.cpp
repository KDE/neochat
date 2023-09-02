// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "mediasizehelper.h"

#include "neochatconfig.h"

MediaSizeHelper::MediaSizeHelper(QObject *parent)
    : QObject(parent)
{
}

qreal MediaSizeHelper::contentMaxWidth() const
{
    return m_contentMaxWidth;
}

void MediaSizeHelper::setContentMaxWidth(qreal contentMaxWidth)
{
    if (contentMaxWidth < 0.0 || qFuzzyCompare(contentMaxWidth, 0.0)) {
        m_contentMaxWidth = -1.0;
        Q_EMIT contentMaxWidthChanged();
        Q_EMIT currentSizeChanged();
        return;
    }
    if (qFuzzyCompare(contentMaxWidth, m_contentMaxWidth)) {
        return;
    }
    m_contentMaxWidth = contentMaxWidth;
    Q_EMIT contentMaxWidthChanged();
    Q_EMIT currentSizeChanged();
}

qreal MediaSizeHelper::contentMaxHeight() const
{
    return m_contentMaxHeight;
}

void MediaSizeHelper::setContentMaxHeight(qreal contentMaxHeight)
{
    if (contentMaxHeight < 0.0 || qFuzzyCompare(contentMaxHeight, 0.0)) {
        m_contentMaxHeight = -1.0;
        Q_EMIT contentMaxHeightChanged();
        Q_EMIT currentSizeChanged();
        return;
    }
    if (qFuzzyCompare(contentMaxHeight, m_contentMaxHeight)) {
        return;
    }
    m_contentMaxHeight = contentMaxHeight;
    Q_EMIT contentMaxHeightChanged();
    Q_EMIT currentSizeChanged();
}

qreal MediaSizeHelper::mediaWidth() const
{
    return m_mediaWidth;
}

void MediaSizeHelper::setMediaWidth(qreal mediaWidth)
{
    if (mediaWidth < 0.0 || qFuzzyCompare(mediaWidth, 0.0)) {
        m_mediaWidth = -1.0;
        Q_EMIT mediaWidthChanged();
        Q_EMIT currentSizeChanged();
        return;
    }
    if (qFuzzyCompare(mediaWidth, m_mediaWidth)) {
        return;
    }
    m_mediaWidth = mediaWidth;
    Q_EMIT mediaWidthChanged();
    Q_EMIT currentSizeChanged();
}

qreal MediaSizeHelper::mediaHeight() const
{
    return m_mediaHeight;
}

void MediaSizeHelper::setMediaHeight(qreal mediaHeight)
{
    if (mediaHeight < 0.0 || qFuzzyCompare(mediaHeight, 0.0)) {
        m_mediaHeight = -1.0;
        Q_EMIT mediaHeightChanged();
        Q_EMIT currentSizeChanged();
        return;
    }
    if (qFuzzyCompare(mediaHeight, m_mediaHeight)) {
        return;
    }
    m_mediaHeight = mediaHeight;
    Q_EMIT mediaHeightChanged();
    Q_EMIT currentSizeChanged();
}

qreal MediaSizeHelper::resolvedMediaWidth() const
{
    if (m_mediaWidth > 0.0) {
        return m_mediaWidth;
    }
    return widthLimit();
}

qreal MediaSizeHelper::resolvedMediaHeight() const
{
    if (m_mediaHeight > 0.0) {
        return m_mediaHeight;
    }
    return widthLimit() / 16.0 * 9.0;
}

qreal MediaSizeHelper::aspectRatio() const
{
    return resolvedMediaWidth() / resolvedMediaHeight();
}

bool MediaSizeHelper::limitWidth() const
{
    // If actual data isn't available we'll be using a placeholder that is width
    // limited so return true.
    if (m_mediaWidth < 0.0 || m_mediaHeight < 0.0) {
        return true;
    }
    return m_mediaWidth >= m_mediaHeight;
}

qreal MediaSizeHelper::widthLimit() const
{
    if (m_contentMaxWidth < 0.0) {
        return NeoChatConfig::self()->mediaMaxWidth();
    }
    return std::min(m_contentMaxWidth, qreal(NeoChatConfig::self()->mediaMaxWidth()));
}

qreal MediaSizeHelper::heightLimit() const
{
    if (m_contentMaxHeight < 0.0) {
        return NeoChatConfig::self()->mediaMaxHeight();
    }
    return std::min(m_contentMaxHeight, qreal(NeoChatConfig::self()->mediaMaxHeight()));
}

QSize MediaSizeHelper::currentSize() const
{
    if (limitWidth()) {
        qreal width = std::min(widthLimit(), resolvedMediaWidth());
        qreal height = width / aspectRatio();
        if (height > heightLimit()) {
            return QSize(qRound(heightLimit() * aspectRatio()), qRound(heightLimit()));
        }
        return QSize(qRound(width), qRound(height));
    } else {
        qreal height = std::min(heightLimit(), resolvedMediaHeight());
        qreal width = height * aspectRatio();
        if (width > widthLimit()) {
            return QSize(qRound(widthLimit()), qRound(widthLimit() / aspectRatio()));
        }
        return QSize(qRound(width), qRound(height));
    }
}

#include "moc_mediasizehelper.cpp"
