// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "delegatesizehelper.h"

#include <cmath>

#include <QDebug>

DelegateSizeHelper::DelegateSizeHelper(QObject *parent)
    : QObject(parent)
{
}

qreal DelegateSizeHelper::parentWidth() const
{
    return m_parentWidth;
}

void DelegateSizeHelper::setParentWidth(qreal parentWidth)
{
    if (parentWidth == m_parentWidth) {
        return;
    }
    m_parentWidth = parentWidth;
    Q_EMIT parentWidthChanged();
    Q_EMIT currentPercentageWidthChanged();
    Q_EMIT currentWidthChanged();
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
}

qreal DelegateSizeHelper::maxWidth() const
{
    return m_maxWidth;
}

void DelegateSizeHelper::setMaxWidth(qreal maxWidth)
{
    if (maxWidth == m_maxWidth) {
        return;
    }
    m_maxWidth = maxWidth;
    Q_EMIT maxWidthChanged();
}

int DelegateSizeHelper::calculateCurrentPercentageWidth() const
{
    // Don't do anything if m_parentWidth hasn't been set yet.
    if (m_parentWidth < 0) {
        return -1;
    }
    // Don't bother with calculations for a horizontal line.
    if (m_startPercentWidth == m_endPercentWidth) {
        return m_startPercentWidth;
    }
    // Dividing by zero is a bad idea.
    if (m_startBreakpoint == m_endBreakpoint) {
        qWarning() << "DelegateSizeHelper::calculateCurrentPercentageWidth() - m_startBreakpoint is equal to m_endBreakpoint this would lead to divide by "
                      "zero, aborting";
        return -1;
    }

    // Fit to y = mx + c
    qreal m = (m_endPercentWidth - m_startPercentWidth) / (m_endBreakpoint - m_startBreakpoint);
    qreal c = m_startPercentWidth - m * m_startBreakpoint;

    // This allows us to clamp correctly if the start or end width is bigger.
    bool endPercentBigger = m_endPercentWidth > m_startPercentWidth;
    int maxPercentWidth = endPercentBigger ? m_endPercentWidth : m_startPercentWidth;
    int minPercentWidth = endPercentBigger ? m_startPercentWidth : m_endPercentWidth;

    int calcPercentWidth = std::round(m * m_parentWidth + c);
    return std::clamp(calcPercentWidth, minPercentWidth, maxPercentWidth);
}

int DelegateSizeHelper::currentPercentageWidth() const
{
    return calculateCurrentPercentageWidth();
}

qreal DelegateSizeHelper::currentWidth() const
{
    if (m_parentWidth < 0) {
        return 0.0;
    }
    int percentWidth = calculateCurrentPercentageWidth();
    // - 1 means bad input values so don't try to calculate.
    if (percentWidth == -1) {
        return 0.0;
    }

    qreal absoluteWidth = m_parentWidth * percentWidth * 0.01;
    if (m_maxWidth < 0.0) {
        return std::round(absoluteWidth);
    } else {
        return std::round(std::min(absoluteWidth, m_maxWidth));
    }
}

#include "moc_delegatesizehelper.cpp"
