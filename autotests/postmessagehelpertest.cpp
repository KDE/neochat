// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QScopeGuard>
#include <QTest>
#include <Quotient/events/event.h>

#include "testutils.h"

#include "blockcache.h"
#include "models/eventmessagecontentmodel.h"
#include "postmessagehelper.h"

using namespace Quotient;

class PostMessageHelperTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestUtils::TestRoom *room = nullptr;
    TestUtils::TestRoom *codeRoom = nullptr;

private Q_SLOTS:
    void initTestCase();

    void sendMessageTest();
    void editMessageTest();
    void threadMessageTest();
    void editCodeBlockPlainTextTest_data();
    void editCodeBlockPlainTextTest();
    void extendCodeBlockPlainTextTest();
};

namespace
{
event_ptr_tt<RoomMessageEvent> makeHtmlMessage(const QString &eventId, const QString &body, const QString &html)
{
    auto event = makeEvent<RoomMessageEvent>(body, RoomMessageEvent::MsgType::Text, std::make_unique<EventContent::TextContent>(html, u"text/html"_s));
    event->addId(eventId);
    event->setSender(u"@bob:kde.org"_s);
    return event;
}
}

void PostMessageHelperTest::initTestCase()
{
    connection = Connection::makeMockConnection(u"@bob:kde.org"_s);
    room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s, u"test-min-sync.json"_s);

    codeRoom = new TestUtils::TestRoom(connection, u"#coderoom:kde.org"_s);
    RoomEvents events;
    events.push_back(makeHtmlMessage(u"$code:example.org"_s,
                                     u"```\nthis/component: foo now bars\nfoo should always bar\n```"_s,
                                     u"<pre><code>this/component: foo now bars\nfoo should always bar\n</code></pre>"_s));
    events.push_back(makeHtmlMessage(u"$codelanguage:example.org"_s,
                                     u"look:\n\n```cpp\nint a;\n\nint b;\n```\n\nafter"_s,
                                     u"<p>look:</p>\n<pre><code class=\"language-cpp\">int a;\n\nint b;\n</code></pre>\n<p>after</p>"_s));
    codeRoom->syncNewEvents(std::move(events));
}

void PostMessageHelperTest::sendMessageTest()
{
    const auto numPending = room->pendingEvents().size();

    Blocks::Cache cache;
    cache.append(std::make_unique<Blocks::TextCacheItem>(Blocks::Text, QTextDocumentFragment::fromMarkdown(u"test message"_s)));

    auto helper = PostMessageHelper(this);
    helper.setRoom(room);
    helper.setCache(&cache);

    helper.postMessage();
    QCOMPARE(room->pendingEvents().size(), numPending + 1);
    const auto pendingEvt = eventCast<const RoomMessageEvent>(room->pendingEvents()[room->pendingEvents().size() - 1].event());
    QCOMPARE(pendingEvt != nullptr, true);
    QCOMPARE(pendingEvt->plainBody(), u"test message"_s);
}

void PostMessageHelperTest::editMessageTest()
{
    const auto numPending = room->pendingEvents().size();

    QCOMPARE(room->messageEvents().size(), 1);
    const auto editEvent = room->findInTimeline("$153456789:example.org"_L1);
    QCOMPARE(editEvent != room->historyEdge(), true);

    Blocks::Cache cache;
    cache.append(std::make_unique<Blocks::TextCacheItem>(Blocks::Text, QTextDocumentFragment::fromMarkdown(u"This is an example text message edited"_s)));

    auto helper = PostMessageHelper(this);
    helper.setRoom(room);
    helper.setCache(&cache);
    helper.setEditId("$153456789:example.org"_L1);

    helper.postMessage();
    QCOMPARE(room->pendingEvents().size(), numPending + 1);
    const auto pendingEvt = eventCast<const RoomMessageEvent>(room->pendingEvents()[room->pendingEvents().size() - 1].event());
    QCOMPARE(pendingEvt != nullptr, true);
    QCOMPARE(pendingEvt->plainBody(), u"* This is an example text message edited"_s);
    QCOMPARE(pendingEvt->relatesTo().has_value(), true);
    QCOMPARE(pendingEvt->relatesTo()->eventId, "$153456789:example.org"_L1);
    QCOMPARE(pendingEvt->relatesTo()->type, "m.replace"_L1);
}

void PostMessageHelperTest::threadMessageTest()
{
    const auto numPending = room->pendingEvents().size();

    Blocks::Cache cache;
    cache.append(std::make_unique<Blocks::TextCacheItem>(Blocks::Text, QTextDocumentFragment::fromMarkdown(u"thread message"_s)));

    auto helper = PostMessageHelper(this);
    helper.setRoom(room);
    helper.setCache(&cache);
    helper.setThreadRootId("$153456789:example.org"_L1);

    helper.postMessage();
    QCOMPARE(room->pendingEvents().size(), numPending + 1);
    const auto pendingEvt = eventCast<const RoomMessageEvent>(room->pendingEvents()[room->pendingEvents().size() - 1].event());
    QCOMPARE(pendingEvt != nullptr, true);
    QCOMPARE(pendingEvt->plainBody(), u"thread message"_s);
    QCOMPARE(pendingEvt->relatesTo().has_value(), true);
    QCOMPARE(pendingEvt->relatesTo()->eventId, "$153456789:example.org"_L1);
    QCOMPARE(pendingEvt->relatesTo()->type, "m.thread"_L1);
}

void PostMessageHelperTest::editCodeBlockPlainTextTest_data()
{
    QTest::addColumn<QString>("eventId");
    QTest::addColumn<QString>("editText");
    QTest::addColumn<QString>("formattedBody");

    QTest::newRow("code block") << u"$code:example.org"_s << u"```\nthis/component: foo now bars\nfoo should always bar\n```"_s
                                << u"<pre><code>this/component: foo now bars\nfoo should always bar\n</code></pre>"_s;
    QTest::newRow("code block with language and text") << u"$codelanguage:example.org"_s << u"look:\n\n```cpp\nint a;\n\nint b;\n```\n\nafter"_s
                                                       << u"<p>look:</p>\n<pre><code class=\"language-cpp\">int a;\n\nint b;\n</code></pre>\n<p>after</p>"_s;
}

void PostMessageHelperTest::editCodeBlockPlainTextTest()
{
    QFETCH(QString, eventId);
    QFETCH(QString, editText);
    QFETCH(QString, formattedBody);

    // https://bugs.kde.org/show_bug.cgi?id=518454
    EventMessageContentModel::richTextActive = false;
    Blocks::CacheItem::richTextActive = false;
    const auto resetRichText = qScopeGuard([] {
        EventMessageContentModel::richTextActive = true;
        Blocks::CacheItem::richTextActive = true;
    });

    auto model = EventMessageContentModel(codeRoom, eventId);
    model.editEvent();

    // The code block keeps its lines and language in the plain text chat bar.
    const auto cache = codeRoom->editCache();
    QVERIFY(cache->at(0) != nullptr);
    QCOMPARE(cache->at(0)->type, Blocks::Text);
    QCOMPARE(cache->at(0)->toString(), editText);
    QCOMPARE(cache->at(1), nullptr);

    // Sending the edit unchanged gives back the same message.
    const auto numPending = codeRoom->pendingEvents().size();
    auto helper = PostMessageHelper(this);
    helper.setRoom(codeRoom);
    helper.setCache(cache);
    helper.setEditId(eventId);
    helper.postMessage();
    QCOMPARE(codeRoom->pendingEvents().size(), numPending + 1);
    const auto newContent = codeRoom->pendingEvents().back()->contentJson()[NewContentKey].toObject();
    QCOMPARE(newContent["body"_L1].toString(), editText);
    QCOMPARE(newContent["formatted_body"_L1].toString(), formattedBody);
}

void PostMessageHelperTest::extendCodeBlockPlainTextTest()
{
    // The steps from https://bugs.kde.org/show_bug.cgi?id=518454: add lines to the end of the code block.
    EventMessageContentModel::richTextActive = false;
    Blocks::CacheItem::richTextActive = false;
    const auto resetRichText = qScopeGuard([] {
        EventMessageContentModel::richTextActive = true;
        Blocks::CacheItem::richTextActive = true;
    });

    auto model = EventMessageContentModel(codeRoom, u"$code:example.org"_s);
    model.editEvent();
    auto cache = codeRoom->editCache();
    auto editText = cache->at(0)->toString();
    editText.insert(editText.lastIndexOf(u"```"_s), u"BUG: 69420\nFIXED-IN: 6.6.6\n"_s);
    cache->clear();
    cache->append(std::make_unique<Blocks::TextCacheItem>(Blocks::Text, QTextDocumentFragment::fromPlainText(editText)));

    auto helper = PostMessageHelper(this);
    helper.setRoom(codeRoom);
    helper.setCache(cache);
    helper.setEditId(u"$code:example.org"_s);
    helper.postMessage();
    const auto newContent = codeRoom->pendingEvents().back()->contentJson()[NewContentKey].toObject();
    QCOMPARE(newContent["formatted_body"_L1].toString(),
             u"<pre><code>this/component: foo now bars\nfoo should always bar\nBUG: 69420\nFIXED-IN: 6.6.6\n</code></pre>"_s);
}

QTEST_MAIN(PostMessageHelperTest)
#include "postmessagehelpertest.moc"
