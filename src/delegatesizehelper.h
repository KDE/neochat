// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

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

    /**
     * @brief The width of the component's parent.
     */
    Q_PROPERTY(qreal parentWidth READ parentWidth WRITE setParentWidth NOTIFY parentWidthChanged)

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
    Q_PROPERTY(int currentPercentageWidth READ currentPercentageWidth NOTIFY currentPercentageWidthChanged)

    /**
     * @brief The width (in px) of the component at the current parentWidth.
     *
     * Will return 0.0 if no parentWidth is set.
     *
     * @sa parentWidth
     */
    Q_PROPERTY(qreal currentWidth READ currentWidth NOTIFY currentWidthChanged)

public:
    DelegateSizeHelper(QObject *parent = nullptr);

    qreal parentWidth() const;
    void setParentWidth(qreal parentWidth);

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

    int currentPercentageWidth() const;

    qreal currentWidth() const;

Q_SIGNALS:
    void parentWidthChanged();
    void startBreakpointChanged();
    void endBreakpointChanged();
    void startPercentWidthChanged();
    void endPercentWidthChanged();
    void maxWidthChanged();
    void currentPercentageWidthChanged();
    void currentWidthChanged();

private:
    qreal m_parentWidth = -1.0;
    qreal m_startBreakpoint;
    qreal m_endBreakpoint;
    int m_startPercentWidth;
    int m_endPercentWidth;
    qreal m_maxWidth = -1.0;

    int calculateCurrentPercentageWidth() const;
};
