// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>
#include <qtestcase.h>

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
    QTest::addColumn<bool>("isPlain");
    QTest::addColumn<QString>("outputstring");

    QTest::newRow("plainText") << u"test string"_s << MessageComponentType::Text << false << u"test string"_s;
    QTest::newRow("list") << u"- list 1\n- list 2\n- list 3\n"_s << MessageComponentType::Text << false << u"- list 1\n- list 2\n- list 3"_s;
    QTest::newRow("code") << u"for (some code) {\n\n    do something\n\n}"_s << MessageComponentType::Code << true
                          << u"```\nfor (some code) {\n    do something\n}\n```"_s;
    QTest::newRow("code with markdown") << u"some code\n\n# looks like a markdown header but is plain in code block"_s << MessageComponentType::Code << true
                                        << u"```\nsome code\n# looks like a markdown header but is plain in code block\n```"_s;
    QTest::newRow("quote") << u"“this is a quote”"_s << MessageComponentType::Quote << false << u"> this is a quote"_s;
    QTest::newRow("heading") << u"# heading\n\nnext line"_s << MessageComponentType::Text << false << u"# heading\n\nnext line"_s;
    QTest::newRow("long string")
        << u"a very very very very very very very very very very very very very very very very very very very very very very very long string"_s
        << MessageComponentType::Text << false
        << u"a very very very very very very very very very very very very very very very very very very very very very very very long string"_s;
    QTest::newRow("links")
        << u"https://bugs.kde.org/show_bug.cgi?id=517208\nhttps://bugs.kde.org/show_bug.cgi?id=517208\nhttps://bugs.kde.org/show_bug.cgi?id=517208\nhttps://bugs.kde.org/show_bug.cgi?id=517208"_s
        << MessageComponentType::Text << true
        << u"https://bugs.kde.org/show_bug.cgi?id=517208\n\nhttps://bugs.kde.org/show_bug.cgi?id=517208\n\nhttps://bugs.kde.org/show_bug.cgi?id=517208\n\nhttps://bugs.kde.org/show_bug.cgi?id=517208"_s;
    QTest::newRow("empty blocks") << u"test\n\n\n\nstring"_s << MessageComponentType::Text << true << u"test\n\nstring"_s;
}

void BlockCacheTest::toStringTest()
{
    QFETCH(QString, inputString);
    QFETCH(MessageComponentType::Type, itemType);
    QFETCH(bool, isPlain);
    QFETCH(QString, outputstring);

    Cache cache;
    cache += CacheItem{
        .type = itemType,
        .content = isPlain ? QTextDocumentFragment::fromPlainText(inputString) : QTextDocumentFragment::fromMarkdown(inputString),
    };

    QCOMPARE(cache.toString(), outputstring);
}

QTEST_MAIN(BlockCacheTest)
#include "blockcachetest.moc"
