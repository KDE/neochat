// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>
#include <Quotient/events/event.h>

#include "testutils.h"

#include "blockcache.h"
#include "postmessagehelper.h"

using namespace Quotient;

class PostMessageHelperTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestUtils::TestRoom *room = nullptr;

private Q_SLOTS:
    void initTestCase();

    void sendMessageTest();
    void editMessageTest();
    void threadMessageTest();
};

void PostMessageHelperTest::initTestCase()
{
    connection = Connection::makeMockConnection(u"@bob:kde.org"_s);
    room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s, u"test-min-sync.json"_s);
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

QTEST_MAIN(PostMessageHelperTest)
#include "postmessagehelpertest.moc"
