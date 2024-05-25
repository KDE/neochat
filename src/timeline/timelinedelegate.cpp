// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "timelinedelegate.h"

TimelineDelegate::TimelineDelegate(QQuickItem *parent)
    : QQuickItem(parent)
{
}

QQuickItem *TimelineDelegate::contentItem()
{
    return m_contentItem;
}

void TimelineDelegate::setContentItem(QQuickItem *item)
{
    if (m_contentItem == item) {
        return;
    }

    if (m_contentItem) {
        disconnect(m_contentItem, &QQuickItem::implicitHeightChanged, this, &TimelineDelegate::updateImplicitHeight);
        m_contentItem->setParentItem(nullptr);
    }

    m_contentItem = item;

    if (m_contentItem) {
        m_contentItem->setParentItem(this);
        connect(m_contentItem, &QQuickItem::implicitHeightChanged, this, &TimelineDelegate::updateImplicitHeight);
    }

    Q_EMIT contentItemChanged();

    updateImplicitHeight();
    resizeContent();
}

bool TimelineDelegate::alwaysFillWidth()
{
    return m_alwaysFillWidth;
}

void TimelineDelegate::setAlwaysFillWidth(bool alwaysFillWidth)
{
    if (alwaysFillWidth == m_alwaysFillWidth) {
        return;
    }
    m_alwaysFillWidth = alwaysFillWidth;
    Q_EMIT alwaysFillWidthChanged();

    resizeContent();
    updatePolish();
}

qreal TimelineDelegate::leftPadding()
{
    return m_leftPadding;
}

void TimelineDelegate::setLeftPadding(qreal leftPadding)
{
    if (qFuzzyCompare(leftPadding, m_leftPadding)) {
        return;
    }

    m_leftPadding = leftPadding;
    Q_EMIT leftPaddingChanged();

    resizeContent();
    updatePolish();
}

qreal TimelineDelegate::rightPadding()
{
    return m_rightPadding;
}

void TimelineDelegate::setRightPadding(qreal rightPadding)
{
    if (qFuzzyCompare(rightPadding, m_rightPadding)) {
        return;
    }

    m_rightPadding = rightPadding;
    Q_EMIT rightPaddingChanged();

    resizeContent();
    updatePolish();
}

void TimelineDelegate::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry == oldGeometry) {
        return;
    }

    QQuickItem::geometryChange(newGeometry, oldGeometry);
    resizeContent();
}

void TimelineDelegate::componentComplete()
{
    QQuickItem::componentComplete();

    auto engine = qmlEngine(this);
    Q_ASSERT(engine);
    m_units = engine->singletonInstance<Kirigami::Platform::Units *>("org.kde.kirigami.platform", "Units");
    Q_ASSERT(m_units);
    setCurveValues();
    connect(m_units, &Kirigami::Platform::Units::gridUnitChanged, this, &TimelineDelegate::setCurveValues);
}

void TimelineDelegate::setCurveValues()
{
    m_leftPadding = qreal(m_units->largeSpacing());
    m_rightPadding = qreal(m_units->largeSpacing());

    m_startBreakpoint = qreal(m_units->gridUnit() * 46);
    m_endBreakpoint = qreal(m_units->gridUnit() * 66);
    m_maxWidth = qreal(m_units->gridUnit() * 60);

    resizeContent();
}

int TimelineDelegate::availablePercentageWidth() const
{
    // Don't bother with calculations for a horizontal line.
    if (m_startPercentWidth == m_endPercentWidth) {
        return m_startPercentWidth;
    }
    // Dividing by zero is a bad idea.
    if (m_startBreakpoint == m_endBreakpoint || qFuzzyCompare(width(), 0)) {
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

qreal TimelineDelegate::maxAvailableWidth() const
{
    if (qFuzzyCompare(width(), 0)) {
        return 0;
    }

    return std::max(width() - m_leftPadding - m_rightPadding, 0.0);
}

qreal TimelineDelegate::availableWidth() const
{
    if (m_alwaysFillWidth) {
        return maxAvailableWidth();
    }

    qreal absoluteWidth = maxAvailableWidth() * availablePercentageWidth() * 0.01;
    return std::round(std::min(absoluteWidth, m_maxWidth));
}

void TimelineDelegate::resizeContent()
{
    if (m_contentItem == nullptr || !isComponentComplete()) {
        return;
    }

    const auto leftPadding = m_leftPadding + (maxAvailableWidth() - availableWidth()) / 2;
    m_contentItem->setPosition(QPointF(leftPadding, 0));
    m_contentItem->setSize(QSizeF(availableWidth(), m_contentItem->implicitHeight()));
}

void TimelineDelegate::updateImplicitHeight()
{
    if (m_contentItem == nullptr) {
        setImplicitHeight(0);
    }
    setImplicitHeight(m_contentItem->implicitHeight());
}

#include "moc_timelinedelegate.cpp"
