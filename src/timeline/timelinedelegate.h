// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef TIMELINEDELEGATE_H
#define TIMELINEDELEGATE_H

#include <QQuickItem>

#include <Kirigami/Platform/Units>

#include "delegatesizehelper.h"

/**
 * @brief The base Item for all delegates in the timeline.
 *
 * This component handles the placing of the main content for a delegate in the
 * timeline. The component is designed for all delegates, positioning them in the
 * timeline with variable padding depending on the window width.
 *
 * This component also supports always setting the delegate to fill the available
 * width in the timeline, e.g. in compact mode.
 */
class TimelineDelegate : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief This property holds the visual content Item.
     *
     * It will be resized both in width and height with the layout resizing.
     * Its height will be resized to still have room for the heder and footer
     */
    Q_PROPERTY(QQuickItem *contentItem READ contentItem WRITE setContentItem NOTIFY contentItemChanged FINAL)

    /**
     * @brief Whether the contentItem should fill all available width regardless of how wide the delegate is.
     *
     * The left and right padding values will still be respected.
     */
    Q_PROPERTY(bool alwaysFillWidth READ alwaysFillWidth WRITE setAlwaysFillWidth NOTIFY alwaysFillWidthChanged FINAL)

    /**
     * @brief The minimum padding to the left of the content.
     */
    Q_PROPERTY(qreal leftPadding READ leftPadding WRITE setLeftPadding NOTIFY leftPaddingChanged FINAL)

    /**
     * @brief The minimum padding to the right of the content.
     */
    Q_PROPERTY(qreal rightPadding READ rightPadding WRITE setRightPadding NOTIFY rightPaddingChanged FINAL)

    /**
     * @brief The width of the timeline column.
     */
    Q_PROPERTY(qreal timelineWidth READ timelineWidth NOTIFY timelineWidthChanged FINAL)

public:
    TimelineDelegate(QQuickItem *parent = nullptr);

    [[nodiscard]] QQuickItem *contentItem();
    void setContentItem(QQuickItem *item);

    [[nodiscard]] bool alwaysFillWidth();
    void setAlwaysFillWidth(bool alwaysFillWidth);

    [[nodiscard]] qreal leftPadding();
    void setLeftPadding(qreal leftPadding);

    [[nodiscard]] qreal rightPadding();
    void setRightPadding(qreal rightPadding);

    [[nodiscard]] qreal timelineWidth();

Q_SIGNALS:
    void contentItemChanged();
    void alwaysFillWidthChanged();
    void leftPaddingChanged();
    void rightPaddingChanged();
    void timelineWidthChanged();

protected:
    Kirigami::Platform::Units *m_units = nullptr;
    virtual void setCurveValues();

    DelegateSizeHelper m_sizeHelper;
    bool m_alwaysFillWidth = false;

    QPointer<QQuickItem> m_contentItem;

    void markAsDirty();
    bool m_isDirty = false;
    virtual void updateImplicitHeight();
    virtual void resizeContent();
    bool m_resizingContent = false;

private:
    void componentComplete() override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void updatePolish() override;
};

#endif
