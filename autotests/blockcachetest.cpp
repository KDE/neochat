// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>
<<<<<<< HEAD
#include <qtestcase.h>
=======
#include <QTextCursor>
#include <QTextList>
>>>>>>> c7096180b (Make sure that lists are not flattened)

#include "blockcache.h"

#include "enums/messagecomponenttype.h"

using namespace Block;

class BlockCacheTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void toStringTest_data();
    void toStringTest();

    void listTest();
};

void BlockCacheTest::toStringTest_data()
{
    QTest::addColumn<QString>("inputString");
    QTest::addColumn<MessageComponentType::Type>("itemType");
    QTest::addColumn<bool>("isPlain");
    QTest::addColumn<QString>("outputstring");

    QTest::newRow("plainText") << u"test string"_s << MessageComponentType::Text << false << u"test string"_s;
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

void BlockCacheTest::listTest()
{
    auto doc = QTextDocument();
    auto cursor = QTextCursor(&doc);
    Cache cache;

    // Single level
    auto listFormat = QTextListFormat();
    listFormat.setStyle(QTextListFormat::ListDecimal);
    cursor.createList(listFormat);
    cursor.insertText(u"list 1\nlist 2\nlist 3"_s);

    cursor.select(QTextCursor::Document);

    cache += CacheItem{
        .type = MessageComponentType::Text,
        .content = cursor.selection(),
    };
    // Note looks weird but from a spec perspective doesn't matter only the first
    // number counts then the rest just go up from there.
    QCOMPARE(cache.toString(), u"1.  list 1\n1.  list 2\n1.  list 3"_s);

    cursor.select(QTextCursor::Document);
    cursor.removeSelectedText();
    cache.clear();

    // Nested
    listFormat = QTextListFormat();
    listFormat.setStyle(QTextListFormat::ListDecimal);
    cursor.createList(listFormat);
    cursor.insertText(u"list 1\n"_s);
    auto currentList = cursor.currentList();
    listFormat.setIndent(listFormat.indent() + 1);
    currentList->setFormat(listFormat);
    cursor.insertText(u"list 1.1"_s);

    cursor.select(QTextCursor::Document);

    cache += CacheItem{
        .type = MessageComponentType::Text,
        .content = cursor.selection(),
    };

    QCOMPARE(cache.toString(), u"1.  list 1\n    1.  list 1.1"_s);
}

QTEST_MAIN(BlockCacheTest)
#include "blockcachetest.moc"
