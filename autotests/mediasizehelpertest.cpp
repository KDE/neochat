// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>
#include <qglobal.h>
#include <qtestcase.h>

#include <cmath>

#include "mediasizehelper.h"
#include "neochatconfig.h"

class MediaSizeHelperTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void uninitialized();

    void limits_data();
    void limits();
};

void MediaSizeHelperTest::uninitialized()
{
    MediaSizeHelper mediasizehelper;
    QCOMPARE(mediasizehelper.currentSize(), QSize(540, qRound(qreal(NeoChatConfig::self()->mediaMaxWidth()) / qreal(16.0) * qreal(9.0))));
}

void MediaSizeHelperTest::limits_data()
{
    QTest::addColumn<qreal>("mediaWidth");
    QTest::addColumn<qreal>("mediaHeight");
    QTest::addColumn<qreal>("contentMaxWidth");
    QTest::addColumn<qreal>("contentMaxHeight");
    QTest::addColumn<QSize>("currentSize");

    QTest::newRow("media smaller than content limits") << qreal(200) << qreal(150) << qreal(400) << qreal(900) << QSize(200, 150);
    QTest::newRow("media smaller than max limits") << qreal(200) << qreal(150) << qreal(-1) << qreal(-1) << QSize(200, 150);

    QTest::newRow("limit by max width") << qreal(600) << qreal(50) << qreal(-1) << qreal(-1) << QSize(540, qRound(qreal(540) / (qreal(600) / qreal(50))));
    QTest::newRow("limit by max height") << qreal(50) << qreal(600) << qreal(-1) << qreal(-1) << QSize(qRound(qreal(540) * (qreal(50) / qreal(600))), 540);

    QTest::newRow("limit by content width") << qreal(600) << qreal(50) << qreal(300) << qreal(-1) << QSize(300, qRound(qreal(300) / (qreal(600) / qreal(50))));
    QTest::newRow("limit by content height") << qreal(50) << qreal(600) << qreal(-1) << qreal(300) << QSize(qRound(qreal(300) * (qreal(50) / qreal(600))), 300);

    QTest::newRow("limit by content width tall media")
        << qreal(400) << qreal(600) << qreal(100) << qreal(400) << QSize(100, qRound(qreal(100) / (qreal(400) / qreal(600))));
    QTest::newRow("limit by content height wide media")
        << qreal(1000) << qreal(600) << qreal(400) << qreal(100) << QSize(qRound(qreal(100) * (qreal(1000) / qreal(600))), 100);
}

void MediaSizeHelperTest::limits()
{
    QFETCH(qreal, mediaWidth);
    QFETCH(qreal, mediaHeight);
    QFETCH(qreal, contentMaxWidth);
    QFETCH(qreal, contentMaxHeight);
    QFETCH(QSize, currentSize);

    MediaSizeHelper mediasizehelper;
    mediasizehelper.setMediaWidth(mediaWidth);
    mediasizehelper.setMediaHeight(mediaHeight);
    mediasizehelper.setContentMaxWidth(contentMaxWidth);
    mediasizehelper.setContentMaxHeight(contentMaxHeight);

    QCOMPARE(mediasizehelper.currentSize(), currentSize);
}

QTEST_GUILESS_MAIN(MediaSizeHelperTest)
#include "mediasizehelpertest.moc"
