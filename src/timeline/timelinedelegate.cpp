// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "timelinedelegate.h"

TimelineDelegate::TimelineDelegate(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_sizeHelper.setParentItem(this);
    connect(&m_sizeHelper, &DelegateSizeHelper::leftPaddingChanged, this, [this]() {
        Q_EMIT leftPaddingChanged();
        resizeContent();
        updatePolish();
    });
    connect(&m_sizeHelper, &DelegateSizeHelper::rightPaddingChanged, this, [this]() {
        Q_EMIT rightPaddingChanged();
        resizeContent();
        updatePolish();
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
    m_sizeHelper.setLeftPadding(qreal(m_units->largeSpacing()));
    m_sizeHelper.setRightPadding(qreal(m_units->largeSpacing()));

    m_sizeHelper.setStartBreakpoint(qreal(m_units->gridUnit() * 46));
    m_sizeHelper.setEndBreakpoint(qreal(m_units->gridUnit() * 66));
    m_sizeHelper.setMaxWidth(qreal(m_units->gridUnit() * 60));

    resizeContent();
}

void TimelineDelegate::resizeContent()
{
    if (m_contentItem == nullptr || !isComponentComplete()) {
        return;
    }

    auto availableWidth = m_alwaysFillWidth ? m_sizeHelper.maxAvailableWidth() : m_sizeHelper.availableWidth();

    const auto leftPadding = m_sizeHelper.leftPadding() + (m_sizeHelper.maxAvailableWidth() - availableWidth) / 2;
    m_contentItem->setPosition(QPointF(leftPadding, 0));
    m_contentItem->setSize(QSizeF(availableWidth, m_contentItem->implicitHeight()));
}

void TimelineDelegate::updateImplicitHeight()
{
    if (m_contentItem == nullptr) {
        setImplicitHeight(0);
    }
    setImplicitHeight(m_contentItem->implicitHeight());
}

#include "moc_timelinedelegate.cpp"
