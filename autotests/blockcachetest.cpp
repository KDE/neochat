// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>

#include "blockcache.h"

#include "enums/messagecomponenttype.h"

using namespace Block;

class BlockCacheTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void toStringTest_data();
    void toStringTest();
};

void BlockCacheTest::toStringTest_data()
{
    QTest::addColumn<QString>("inputString");
    QTest::addColumn<MessageComponentType::Type>("itemType");
    QTest::addColumn<QString>("outputstring");

    QTest::newRow("plainText") << u"test string"_s << MessageComponentType::Text << u"test string"_s;
    QTest::newRow("list") << u"- list 1\n- list 2\n- list 3\n"_s << MessageComponentType::Text << u"- list 1\n- list 2\n- list 3"_s;
    QTest::newRow("code") << u"for (some code) {\n\n    do something\n\n}"_s << MessageComponentType::Code
                          << u"```\nfor (some code) {\n    do something\n}\n```"_s;
    QTest::newRow("quote") << u"\"this is a quote\""_s << MessageComponentType::Quote << u"> this is a quote"_s;
    QTest::newRow("heading") << u"# heading\n\nnext line"_s << MessageComponentType::Text << u"# heading\n\nnext line"_s;
}

void BlockCacheTest::toStringTest()
{
    QFETCH(QString, inputString);
    QFETCH(MessageComponentType::Type, itemType);
    QFETCH(QString, outputstring);

    Cache cache;
    cache += CacheItem{
        .type = itemType,
        .content = QTextDocumentFragment::fromMarkdown(inputString),
    };

    QCOMPARE(cache.toString(), outputstring);
}

QTEST_MAIN(BlockCacheTest)
#include "blockcachetest.moc"
