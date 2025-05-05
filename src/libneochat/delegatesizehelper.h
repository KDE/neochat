// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>

/**
 * @class DelegateSizeHelper
 *
 * A class to help calculate the current width of a chat delegate/bar.
 *
 * The aim is to support a dynamic sizing based upon the width of the page. There is
 * a built in curve that allows the width to transition between two percentages based
 * upon a start and finish break point. This is to provide better convergence where
 * generally the delegate will need to fill all or most the screen when thin but
 * should max out in size and only fill a lower percentage of the screen when wide.
 *
 * @note While the main intended usage is to start with a high percentage when the parentWidth
 *       is small and transition to a lower one when large, the math is setup for the
 *       general case so any combination of parameters works.
 */
class DelegateSizeHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The parent item that defines the available content area.
     */
    Q_PROPERTY(QQuickItem *parentItem READ parentItem WRITE setParentItem NOTIFY parentItemChanged)

    /**
     * @brief The amount of padding to be removed from the left of the available content area.
     *
     * The padding is removed before calculating the available width, i.e. max available width
     * at 100% is equal to parent width minus padding.
     */
    Q_PROPERTY(qreal leftPadding READ leftPadding WRITE setLeftPadding NOTIFY leftPaddingChanged)

    /**
     * @brief The amount of padding to be removed from the right of the available content area.
     *
     * The padding is removed before calculating the available width, i.e. max available width
     * at 100% is equal to parent width minus padding.
     */
    Q_PROPERTY(qreal rightPadding READ rightPadding WRITE setRightPadding NOTIFY rightPaddingChanged)

    /**
     * @brief The width (in px) when the width percentage should start to transition.
     */
    Q_PROPERTY(qreal startBreakpoint READ startBreakpoint WRITE setStartBreakpoint NOTIFY startBreakpointChanged)

    /**
     * @brief The width (in px) when the width percentage should finish transitioning.
     */
    Q_PROPERTY(qreal endBreakpoint READ endBreakpoint WRITE setEndBreakpoint NOTIFY endBreakpointChanged)

    /**
     * @brief The width (in %) of the component at or before the startBreakpoint.
     *
     * @sa startBreakpoint
     */
    Q_PROPERTY(int startPercentWidth READ startPercentWidth WRITE setStartPercentWidth NOTIFY startPercentWidthChanged)

    /**
     * @brief The width (in %) of the component at or after the endBreakpoint.
     *
     * @sa endBreakpoint
     */
    Q_PROPERTY(int endPercentWidth READ endPercentWidth WRITE setEndPercentWidth NOTIFY endPercentWidthChanged)

    /**
     * @brief The absolute maximum width (in px) the component can be.
     */
    Q_PROPERTY(qreal maxWidth READ maxWidth WRITE setMaxWidth NOTIFY maxWidthChanged)

    /**
     * @brief The width (in %) of the component at the current parentWidth.
     *
     * Will return -1 if no parentWidth is set or startBreakpoint == endBreakpoint.
     *
     * @sa parentWidth, startBreakpoint, endBreakpoint
     */
    Q_PROPERTY(int availablePercentageWidth READ availablePercentageWidth NOTIFY availableWidthChanged)

    /**
     * @brief The width (in px) of the component at the current parentWidth.
     *
     * Will return 0.0 if no parentWidth is set.
     *
     * @sa parentWidth
     */
    Q_PROPERTY(qreal availableWidth READ availableWidth NOTIFY availableWidthChanged)

public:
    explicit DelegateSizeHelper(QObject *parent = nullptr);

    QQuickItem *parentItem() const;
    void setParentItem(QQuickItem *parentItem);

    qreal leftPadding() const;
    void setLeftPadding(qreal leftPadding);
    qreal rightPadding() const;
    void setRightPadding(qreal rightPadding);

    qreal startBreakpoint() const;
    void setStartBreakpoint(qreal startBreakpoint);
    qreal endBreakpoint() const;
    void setEndBreakpoint(qreal endBreakpoint);

    int startPercentWidth() const;
    void setStartPercentWidth(int startPercentWidth);
    int endPercentWidth() const;
    void setEndPercentWidth(int endPercentWidth);

    qreal maxWidth() const;
    void setMaxWidth(qreal maxWidth);

    qreal maxAvailableWidth() const;

    int availablePercentageWidth() const;
    qreal availableWidth() const;

    /**
     * @brief The left x position of the content column.
     */
    qreal leftX() const;

    /**
     * @brief The right x position of the content column.
     */
    qreal rightX() const;

Q_SIGNALS:
    void parentItemChanged();
    void leftPaddingChanged();
    void rightPaddingChanged();
    void startBreakpointChanged();
    void endBreakpointChanged();
    void startPercentWidthChanged();
    void endPercentWidthChanged();
    void maxWidthChanged();
    void availablePercentageWidthChanged();
    void availableWidthChanged();

private:
    QPointer<QQuickItem> m_parentItem;

    qreal m_leftPadding = 0.0;
    qreal m_rightPadding = 0.0;

    qreal m_startBreakpoint = 0.0;
    qreal m_endBreakpoint = 0.0;
    int m_startPercentWidth = 100;
    int m_endPercentWidth = 85;
    std::optional<qreal> m_maxWidth = std::nullopt;

    void calcWidthsChanged();

    int calculateAvailablePercentageWidth() const;
};
