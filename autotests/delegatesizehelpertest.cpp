// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>

#include "delegatesizehelper.h"

class DelegateSizeHelperTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void risingPercentage_data();
    void risingPercentage();

    void fallingPercentage_data();
    void fallingPercentage();

    void equalPercentage_data();
    void equalPercentage();

    void equalBreakpoint_data();
    void equalBreakpoint();
};

void DelegateSizeHelperTest::risingPercentage_data()
{
    QTest::addColumn<qreal>("parentWidth");
    QTest::addColumn<int>("currentPercentageWidth");
    QTest::addColumn<qreal>("currentWidth");

    QTest::newRow("zero") << qreal(0) << int(0) << qreal(0);
    QTest::newRow("one hundred") << qreal(100) << int(0) << qreal(0);
    QTest::newRow("one fifty") << qreal(150) << int(50) << qreal(75);
    QTest::newRow("two hundred") << qreal(200) << int(100) << qreal(200);
    QTest::newRow("three hundred") << qreal(300) << int(100) << qreal(300);
}

void DelegateSizeHelperTest::risingPercentage()
{
    QFETCH(qreal, parentWidth);
    QFETCH(int, currentPercentageWidth);
    QFETCH(qreal, currentWidth);

    DelegateSizeHelper delegateSizeHelper;
    delegateSizeHelper.setStartBreakpoint(100);
    delegateSizeHelper.setEndBreakpoint(200);
    delegateSizeHelper.setStartPercentWidth(0);
    delegateSizeHelper.setEndPercentWidth(100);

    delegateSizeHelper.setParentWidth(parentWidth);

    QCOMPARE(delegateSizeHelper.currentPercentageWidth(), currentPercentageWidth);
    QCOMPARE(delegateSizeHelper.currentWidth(), currentWidth);
}

void DelegateSizeHelperTest::fallingPercentage_data()
{
    QTest::addColumn<qreal>("parentWidth");
    QTest::addColumn<int>("currentPercentageWidth");
    QTest::addColumn<qreal>("currentWidth");

    QTest::newRow("zero") << qreal(0) << int(100) << qreal(0);
    QTest::newRow("one hundred") << qreal(100) << int(100) << qreal(100);
    QTest::newRow("one fifty") << qreal(150) << int(50) << qreal(75);
    QTest::newRow("two hundred") << qreal(200) << int(0) << qreal(0);
    QTest::newRow("three hundred") << qreal(300) << int(0) << qreal(0);
}

void DelegateSizeHelperTest::fallingPercentage()
{
    QFETCH(qreal, parentWidth);
    QFETCH(int, currentPercentageWidth);
    QFETCH(qreal, currentWidth);

    DelegateSizeHelper delegateSizeHelper;
    delegateSizeHelper.setStartBreakpoint(100);
    delegateSizeHelper.setEndBreakpoint(200);
    delegateSizeHelper.setStartPercentWidth(100);
    delegateSizeHelper.setEndPercentWidth(0);

    delegateSizeHelper.setParentWidth(parentWidth);

    QCOMPARE(delegateSizeHelper.currentPercentageWidth(), currentPercentageWidth);
    QCOMPARE(delegateSizeHelper.currentWidth(), currentWidth);
}

void DelegateSizeHelperTest::equalPercentage_data()
{
    QTest::addColumn<qreal>("parentWidth");
    QTest::addColumn<int>("currentPercentageWidth");
    QTest::addColumn<qreal>("currentWidth");

    QTest::newRow("zero") << qreal(0) << int(50) << qreal(0);
    QTest::newRow("one hundred") << qreal(100) << int(50) << qreal(50);
    QTest::newRow("one fifty") << qreal(150) << int(50) << qreal(75);
    QTest::newRow("two hundred") << qreal(200) << int(50) << qreal(100);
    QTest::newRow("three hundred") << qreal(300) << int(50) << qreal(150);
}

void DelegateSizeHelperTest::equalPercentage()
{
    QFETCH(qreal, parentWidth);
    QFETCH(int, currentPercentageWidth);
    QFETCH(qreal, currentWidth);

    DelegateSizeHelper delegateSizeHelper;
    delegateSizeHelper.setStartBreakpoint(100);
    delegateSizeHelper.setEndBreakpoint(200);
    delegateSizeHelper.setStartPercentWidth(50);
    delegateSizeHelper.setEndPercentWidth(50);

    delegateSizeHelper.setParentWidth(parentWidth);

    QCOMPARE(delegateSizeHelper.currentPercentageWidth(), currentPercentageWidth);
    QCOMPARE(delegateSizeHelper.currentWidth(), currentWidth);
}

void DelegateSizeHelperTest::equalBreakpoint_data()
{
    QTest::addColumn<int>("startPercentageWidth");
    QTest::addColumn<int>("endPercentageWidth");
    QTest::addColumn<int>("currentPercentageWidth");
    QTest::addColumn<qreal>("currentWidth");

    QTest::newRow("start higher") << int(100) << int(0) << int(-1) << qreal(0);
    QTest::newRow("equal") << int(50) << int(50) << int(50) << qreal(500);
    QTest::newRow("end higher") << int(0) << int(100) << int(-1) << qreal(0);
}

/**
 * We expect a default return except in the case where the the two percentages are
 * equal as that case can be calculated without dividing by zero.
 */
void DelegateSizeHelperTest::equalBreakpoint()
{
    QFETCH(int, startPercentageWidth);
    QFETCH(int, endPercentageWidth);
    QFETCH(int, currentPercentageWidth);
    QFETCH(qreal, currentWidth);

    DelegateSizeHelper delegateSizeHelper;
    delegateSizeHelper.setStartBreakpoint(100);
    delegateSizeHelper.setEndBreakpoint(100);
    delegateSizeHelper.setStartPercentWidth(startPercentageWidth);
    delegateSizeHelper.setEndPercentWidth(endPercentageWidth);

    delegateSizeHelper.setParentWidth(1000);

    QCOMPARE(delegateSizeHelper.currentPercentageWidth(), currentPercentageWidth);
    QCOMPARE(delegateSizeHelper.currentWidth(), currentWidth);
}

QTEST_GUILESS_MAIN(DelegateSizeHelperTest)
#include "delegatesizehelpertest.moc"
