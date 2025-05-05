// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "timelinedelegate.h"

TimelineDelegate::TimelineDelegate(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_sizeHelper.setParentItem(this);
    connect(&m_sizeHelper, &DelegateSizeHelper::leftPaddingChanged, this, [this]() {
        Q_EMIT leftPaddingChanged();
        Q_EMIT timelineWidthChanged();
        markAsDirty();
    });
    connect(&m_sizeHelper, &DelegateSizeHelper::rightPaddingChanged, this, [this]() {
        Q_EMIT rightPaddingChanged();
        Q_EMIT timelineWidthChanged();
        markAsDirty();
    });
    connect(&m_sizeHelper, &DelegateSizeHelper::availableWidthChanged, this, [this]() {
        Q_EMIT timelineWidthChanged();
        markAsDirty();
    });
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
        m_contentItem->disconnect(this);
        m_contentItem->setParentItem(nullptr);
    }

    m_contentItem = item;

    if (m_contentItem) {
        m_contentItem->setParentItem(this);
        connect(m_contentItem, &QQuickItem::implicitWidthChanged, this, &TimelineDelegate::markAsDirty);
        connect(m_contentItem, &QQuickItem::implicitHeightChanged, this, &TimelineDelegate::markAsDirty);
        connect(m_contentItem, &QQuickItem::visibleChanged, this, &TimelineDelegate::markAsDirty);
    }

    markAsDirty();
    Q_EMIT contentItemChanged();
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

    if (m_alwaysFillWidth) {
        m_sizeHelper.setEndPercentWidth(100);
    } else {
        m_sizeHelper.setEndPercentWidth(85);
    }

    Q_EMIT alwaysFillWidthChanged();

    markAsDirty();
}

qreal TimelineDelegate::leftPadding()
{
    return m_sizeHelper.leftPadding();
}

void TimelineDelegate::setLeftPadding(qreal leftPadding)
{
    m_sizeHelper.setLeftPadding(leftPadding);
}

qreal TimelineDelegate::rightPadding()
{
    return m_sizeHelper.rightPadding();
}

void TimelineDelegate::setRightPadding(qreal rightPadding)
{
    m_sizeHelper.setRightPadding(rightPadding);
}

qreal TimelineDelegate::timelineWidth()
{
    return m_sizeHelper.availableWidth();
}

void TimelineDelegate::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry != oldGeometry) {
        markAsDirty();
    }

    QQuickItem::geometryChange(newGeometry, oldGeometry);
}

void TimelineDelegate::componentComplete()
{
    QQuickItem::componentComplete();

    auto engine = qmlEngine(this);
    Q_ASSERT(engine);
    m_units = engine->singletonInstance<Kirigami::Platform::Units *>("org.kde.kirigami.platform", "Units");
    Q_ASSERT(m_units);
    connect(m_units, &Kirigami::Platform::Units::gridUnitChanged, this, &TimelineDelegate::setCurveValues);
    connect(m_units, &Kirigami::Platform::Units::largeSpacingChanged, this, &TimelineDelegate::setCurveValues);
    connect(m_units, &Kirigami::Platform::Units::smallSpacingChanged, this, &TimelineDelegate::setCurveValues);
    setCurveValues();

    if (m_isDirty) {
        resizeContent();
    }
}

void TimelineDelegate::setCurveValues()
{
    m_sizeHelper.setLeftPadding(qreal(m_units->largeSpacing()));
    m_sizeHelper.setRightPadding(qreal(m_units->largeSpacing()));

    m_sizeHelper.setStartBreakpoint(qreal(m_units->gridUnit() * 46));
    m_sizeHelper.setEndBreakpoint(qreal(m_units->gridUnit() * 66));
    m_sizeHelper.setMaxWidth(qreal(m_units->gridUnit() * 60));
}

void TimelineDelegate::markAsDirty()
{
    if (!m_isDirty) {
        m_isDirty = true;
        polish();
    }
}

void TimelineDelegate::updatePolish()
{
    if (m_isDirty) {
        resizeContent();
    }
}

void TimelineDelegate::resizeContent()
{
    if (m_contentItem == nullptr || !isComponentComplete() || m_resizingContent) {
        return;
    }

    m_isDirty = false;
    m_resizingContent = true;

    updateImplicitHeight();

    m_contentItem->setPosition(QPointF(m_sizeHelper.leftX(), 0));
    m_contentItem->setSize(QSizeF(m_sizeHelper.availableWidth(), m_contentItem->implicitHeight()));

    m_resizingContent = false;
}

void TimelineDelegate::updateImplicitHeight()
{
    if (m_contentItem == nullptr) {
        setImplicitHeight(0);
    }
    setImplicitHeight(m_contentItem->implicitHeight());
}

#include "moc_timelinedelegate.cpp"
