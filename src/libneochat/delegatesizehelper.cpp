// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "delegatesizehelper.h"

#include <cmath>

#include <QDebug>

DelegateSizeHelper::DelegateSizeHelper(QObject *parent)
    : QObject(parent)
{
}

QQuickItem *DelegateSizeHelper::parentItem() const
{
    return m_parentItem;
}

void DelegateSizeHelper::setParentItem(QQuickItem *parentItem)
{
    if (parentItem == m_parentItem) {
        return;
    }
    m_parentItem = parentItem;

    if (m_parentItem) {
        connect(m_parentItem, &QQuickItem::widthChanged, this, [this]() {
            calcWidthsChanged();
        });
    }

    Q_EMIT parentItemChanged();
    calcWidthsChanged();
}

qreal DelegateSizeHelper::leftPadding() const
{
    return m_leftPadding;
}

void DelegateSizeHelper::setLeftPadding(qreal leftPadding)
{
    if (qFuzzyCompare(leftPadding, m_leftPadding)) {
        return;
    }
    m_leftPadding = leftPadding;
    Q_EMIT leftPaddingChanged();
    calcWidthsChanged();
}

qreal DelegateSizeHelper::rightPadding() const
{
    return m_rightPadding;
}

void DelegateSizeHelper::setRightPadding(qreal rightPadding)
{
    if (qFuzzyCompare(rightPadding, m_rightPadding)) {
        return;
    }
    m_rightPadding = rightPadding;
    Q_EMIT rightPaddingChanged();
    calcWidthsChanged();
}

qreal DelegateSizeHelper::startBreakpoint() const
{
    return m_startBreakpoint;
}

void DelegateSizeHelper::setStartBreakpoint(qreal startBreakpoint)
{
    if (startBreakpoint == m_startBreakpoint) {
        return;
    }
    m_startBreakpoint = startBreakpoint;
    Q_EMIT startBreakpointChanged();
    calcWidthsChanged();
}

qreal DelegateSizeHelper::endBreakpoint() const
{
    return m_endBreakpoint;
}

void DelegateSizeHelper::setEndBreakpoint(qreal endBreakpoint)
{
    if (endBreakpoint == m_endBreakpoint) {
        return;
    }
    m_endBreakpoint = endBreakpoint;
    Q_EMIT endBreakpointChanged();
    calcWidthsChanged();
}

int DelegateSizeHelper::startPercentWidth() const
{
    return m_startPercentWidth;
}

void DelegateSizeHelper::setStartPercentWidth(int startPercentWidth)
{
    if (startPercentWidth == m_startPercentWidth) {
        return;
    }
    m_startPercentWidth = startPercentWidth;
    Q_EMIT startPercentWidthChanged();
    calcWidthsChanged();
}

int DelegateSizeHelper::endPercentWidth() const
{
    return m_endPercentWidth;
}

void DelegateSizeHelper::setEndPercentWidth(int endPercentWidth)
{
    if (endPercentWidth == m_endPercentWidth) {
        return;
    }
    m_endPercentWidth = endPercentWidth;
    Q_EMIT endPercentWidthChanged();
    calcWidthsChanged();
}

qreal DelegateSizeHelper::maxWidth() const
{
    if (m_maxWidth == std::nullopt) {
        if (m_parentItem) {
            return m_parentItem->width();
        }
        return 0.0;
    }

    return *m_maxWidth;
}

void DelegateSizeHelper::setMaxWidth(qreal maxWidth)
{
    if (maxWidth == m_maxWidth) {
        return;
    }
    m_maxWidth = maxWidth;
    Q_EMIT maxWidthChanged();
    calcWidthsChanged();
}

qreal DelegateSizeHelper::maxAvailableWidth() const
{
    if (!m_parentItem || qFuzzyCompare(m_parentItem->width(), 0)) {
        return 0;
    }
    return std::max(m_parentItem->width() - m_leftPadding - m_rightPadding, 0.0);
}

int DelegateSizeHelper::calculateAvailablePercentageWidth() const
{
    // Don't bother with calculations for a horizontal line.
    if (m_startPercentWidth == m_endPercentWidth) {
        return m_startPercentWidth;
    }
    // Dividing by zero is a bad idea.
    if (m_startBreakpoint == m_endBreakpoint || qFuzzyCompare(maxAvailableWidth(), 0)) {
        return 100;
    }

    // Fit to y = mx + c
    qreal m = (m_endPercentWidth - m_startPercentWidth) / (m_endBreakpoint - m_startBreakpoint);
    qreal c = m_startPercentWidth - m * m_startBreakpoint;

    // This allows us to clamp correctly if the start or end width is bigger.
    bool endPercentBigger = m_endPercentWidth > m_startPercentWidth;
    int maxPercentWidth = endPercentBigger ? m_endPercentWidth : m_startPercentWidth;
    int minPercentWidth = endPercentBigger ? m_startPercentWidth : m_endPercentWidth;

    int calcPercentWidth = std::round(m * maxAvailableWidth() + c);
    return std::clamp(calcPercentWidth, minPercentWidth, maxPercentWidth);
}

int DelegateSizeHelper::availablePercentageWidth() const
{
    return calculateAvailablePercentageWidth();
}

qreal DelegateSizeHelper::availableWidth() const
{
    qreal absoluteWidth = maxAvailableWidth() * availablePercentageWidth() * 0.01;
    return std::round(std::min(absoluteWidth, maxWidth()));
}

qreal DelegateSizeHelper::leftX() const
{
    return m_leftPadding + (maxAvailableWidth() - availableWidth()) / 2;
}

qreal DelegateSizeHelper::rightX() const
{
    return m_leftPadding + maxAvailableWidth() - (maxAvailableWidth() - availableWidth()) / 2;
}

void DelegateSizeHelper::calcWidthsChanged()
{
    Q_EMIT availablePercentageWidthChanged();
    Q_EMIT availableWidthChanged();
}

#include "moc_delegatesizehelper.cpp"
