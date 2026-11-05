// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QMimeDatabase>
#include <QObject>
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
    TestUtils::TestRoom *imageRoom = nullptr;

private Q_SLOTS:
    void initTestCase();

    void sendMessageTest();
    void editMessageTest();
    void threadMessageTest();
    void editImageFillsEditCacheTest();
    void editImageCaptionTest();
    void editImageRemoveCaptionTest();
};

void PostMessageHelperTest::initTestCase()
{
    connection = Connection::makeMockConnection(u"@bob:kde.org"_s);
    room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s, u"test-min-sync.json"_s);

    // A room with an image message that has a caption.
    imageRoom = new TestUtils::TestRoom(connection, u"#imageroom:kde.org"_s);
    auto imageEvent = makeEvent<RoomMessageEvent>(u"A picture of a cat"_s,
                                                  RoomMessageEvent::MsgType::Image,
                                                  std::make_unique<EventContent::ImageContent>(QUrl(u"mxc://example.org/JWEIFJgwEIhweiWJE"_s),
                                                                                               31037,
                                                                                               QMimeDatabase().mimeTypeForName(u"image/png"_s),
                                                                                               QSize(300, 200),
                                                                                               u"cat.png"_s));
    imageEvent->addId(u"$imageevent:example.org"_s);
    imageEvent->setSender(u"@bob:kde.org"_s);
    RoomEvents events;
    events.push_back(std::move(imageEvent));
    imageRoom->syncNewEvents(std::move(events));
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

void PostMessageHelperTest::editImageFillsEditCacheTest()
{
    auto model = EventMessageContentModel(imageRoom, u"$imageevent:example.org"_s);
    model.editEvent();

    // The chat bar shows the image followed by its caption.
    const auto cache = imageRoom->editCache();
    QCOMPARE(cache->at(0) != nullptr, true);
    QCOMPARE(cache->at(0)->type, Blocks::Image);
    QCOMPARE(cache->at(1) != nullptr, true);
    QCOMPARE(cache->at(1)->type, Blocks::Text);
    QCOMPARE(cache->at(1)->toString(), u"A picture of a cat"_s);
    QCOMPARE(cache->at(2), nullptr);

    model.cancelEventEdit();
    QVERIFY(cache->empty());
}

void PostMessageHelperTest::editImageCaptionTest()
{
    const auto numPending = imageRoom->pendingEvents().size();

    auto model = EventMessageContentModel(imageRoom, u"$imageevent:example.org"_s);
    model.editEvent();
    auto cache = imageRoom->editCache();
    cache->removeAt(1);
    cache->append(std::make_unique<Blocks::TextCacheItem>(Blocks::Text, QTextDocumentFragment::fromMarkdown(u"A picture of a dog"_s)));

    auto helper = PostMessageHelper(this);
    helper.setRoom(imageRoom);
    helper.setCache(cache);
    helper.setEditId("$imageevent:example.org"_L1);

    helper.postMessage();
    // Only the edit is sent, the image isn't uploaded again.
    QCOMPARE(imageRoom->pendingEvents().size(), numPending + 1);
    const auto pendingEvt = eventCast<const RoomMessageEvent>(imageRoom->pendingEvents()[imageRoom->pendingEvents().size() - 1].event());
    QVERIFY(pendingEvt != nullptr);
    QCOMPARE(pendingEvt->plainBody(), u"* A picture of a dog"_s);
    QVERIFY(pendingEvt->relatesTo().has_value());
    QCOMPARE(pendingEvt->relatesTo()->eventId, "$imageevent:example.org"_L1);
    QCOMPARE(pendingEvt->relatesTo()->type, EventRelation::ReplacementType);

    // The replacement content must still be the image, with only the caption changed.
    const auto newContent = pendingEvt->contentJson()[NewContentKey].toObject();
    QCOMPARE(newContent["msgtype"_L1].toString(), u"m.image"_s);
    QCOMPARE(newContent["body"_L1].toString(), u"A picture of a dog"_s);
    QCOMPARE(newContent["url"_L1].toString(), u"mxc://example.org/JWEIFJgwEIhweiWJE"_s);
    QCOMPARE(newContent["filename"_L1].toString(), u"cat.png"_s);
    QCOMPARE(newContent["info"_L1].toObject()["mimetype"_L1].toString(), u"image/png"_s);
    QVERIFY(!newContent.contains(RelatesToKey));
    // Media captions only have a plain body.
    QVERIFY(!newContent.contains("format"_L1));
    QVERIFY(!newContent.contains("formatted_body"_L1));
    QVERIFY(!pendingEvt->contentJson().contains("formatted_body"_L1));
    QVERIFY(cache->empty());
}

void PostMessageHelperTest::editImageRemoveCaptionTest()
{
    const auto numPending = imageRoom->pendingEvents().size();

    auto model = EventMessageContentModel(imageRoom, u"$imageevent:example.org"_s);
    model.editEvent();
    auto cache = imageRoom->editCache();
    cache->removeAt(1);

    auto helper = PostMessageHelper(this);
    helper.setRoom(imageRoom);
    helper.setCache(cache);
    helper.setEditId("$imageevent:example.org"_L1);

    helper.postMessage();
    QCOMPARE(imageRoom->pendingEvents().size(), numPending + 1);
    const auto pendingEvt = eventCast<const RoomMessageEvent>(imageRoom->pendingEvents()[imageRoom->pendingEvents().size() - 1].event());
    QVERIFY(pendingEvt != nullptr);

    // Without a caption the body is the file name.
    const auto newContent = pendingEvt->contentJson()[NewContentKey].toObject();
    QCOMPARE(newContent["msgtype"_L1].toString(), u"m.image"_s);
    QCOMPARE(newContent["body"_L1].toString(), u"cat.png"_s);
    QCOMPARE(newContent["filename"_L1].toString(), u"cat.png"_s);
    QCOMPARE(newContent["url"_L1].toString(), u"mxc://example.org/JWEIFJgwEIhweiWJE"_s);
    QVERIFY(!newContent.contains("formatted_body"_L1));
}

QTEST_MAIN(PostMessageHelperTest)
#include "postmessagehelpertest.moc"
