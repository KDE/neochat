// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>

#include "eventhandler.h"

#include <KFormat>

#include <Quotient/connection.h>
#include <Quotient/quotient_common.h>
#include <Quotient/syncdata.h>
#include <qcbormap.h>
#include <qtestcase.h>

#include "linkpreviewer.h"
#include "models/reactionmodel.h"
#include "neochatroom.h"
#include "utils.h"

#include "testutils.h"

using namespace Quotient;

class EventHandlerTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestUtils::TestRoom *room = nullptr;

private Q_SLOTS:
    void initTestCase();

    void authorDisplayName();
    void nullAuthorDisplayName();
    void singleLineSidplayName();
    void nullSingleLineDisplayName();
    void time();
    void nullTime();
    void timeString();
    void highlighted();
    void nullHighlighted();
    void hidden();
    void nullHidden();
    void body();
    void nullBody();
    void genericBody_data();
    void genericBody();
    void nullGenericBody();
    void markdownBody();
    void markdownBodyReply();
    void subtitle();
    void nullSubtitle();
    void mediaInfo();
    void nullMediaInfo();
    void replyAuthor();
    void nullReplyAuthor();
    void location();
    void nullLocation();
};

void EventHandlerTest::initTestCase()
{
    connection = Connection::makeMockConnection(u"@bob:kde.org"_s);
    room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s, u"test-eventhandler-sync.json"_s);
}

void EventHandlerTest::authorDisplayName()
{
    QCOMPARE(EventHandler::authorDisplayName(room, room->messageEvents().at(1).get()), u"before"_s);
}

void EventHandlerTest::nullAuthorDisplayName()
{
    QTest::ignoreMessage(QtWarningMsg, "authorDisplayName called with room set to nullptr.");
    QCOMPARE(EventHandler::authorDisplayName(nullptr, nullptr), QString());

    QTest::ignoreMessage(QtWarningMsg, "authorDisplayName called with event set to nullptr.");
    QCOMPARE(EventHandler::authorDisplayName(room, nullptr), QString());
}

void EventHandlerTest::singleLineSidplayName()
{
    QCOMPARE(EventHandler::singleLineAuthorDisplayname(room, room->messageEvents().at(11).get()), "Look at me I put newlines in my display name"_L1);
}

void EventHandlerTest::nullSingleLineDisplayName()
{
    QTest::ignoreMessage(QtWarningMsg, "singleLineAuthorDisplayname called with room set to nullptr.");
    QCOMPARE(EventHandler::singleLineAuthorDisplayname(nullptr, nullptr), QString());

    QTest::ignoreMessage(QtWarningMsg, "singleLineAuthorDisplayname called with event set to nullptr.");
    QCOMPARE(EventHandler::singleLineAuthorDisplayname(room, nullptr), QString());
}

void EventHandlerTest::time()
{
    const auto event = room->messageEvents().at(0).get();

    QCOMPARE(EventHandler::time(room, event), QDateTime::fromMSecsSinceEpoch(1432735824654, QTimeZone(QTimeZone::UTC)));

    const auto txID = room->postJson("m.room.message"_L1, event->fullJson());
    QCOMPARE(room->pendingEvents().size(), 1);
    const auto pendingIt = room->findPendingEvent(txID);
    QCOMPARE(EventHandler::time(room, pendingIt->event(), true), pendingIt->lastUpdated());

    room->discardMessage(txID);
    QCOMPARE(room->pendingEvents().size(), 0);
}

void EventHandlerTest::nullTime()
{
    QTest::ignoreMessage(QtWarningMsg, "time called with room set to nullptr.");
    QCOMPARE(EventHandler::time(nullptr, nullptr), QDateTime());

    QTest::ignoreMessage(QtWarningMsg, "time called with event set to nullptr.");
    QCOMPARE(EventHandler::time(room, nullptr), QDateTime());
}

void EventHandlerTest::timeString()
{
    const auto event = room->messageEvents().at(0).get();

    KFormat format;

    QCOMPARE(EventHandler::timeString(room, event, false),
             QLocale().toString(QDateTime::fromMSecsSinceEpoch(1432735824654, QTimeZone(QTimeZone::UTC)).toLocalTime().time(), QLocale::ShortFormat));
    QCOMPARE(EventHandler::timeString(room, event, true),
             format.formatRelativeDate(QDateTime::fromMSecsSinceEpoch(1432735824654, QTimeZone(QTimeZone::UTC)).toLocalTime().date(), QLocale::ShortFormat));
    QCOMPARE(EventHandler::timeString(room, event, u"hh:mm"_s), QDateTime::fromMSecsSinceEpoch(1432735824654, QTimeZone(QTimeZone::UTC)).toString(u"hh:mm"_s));

    const auto txID = room->postJson("m.room.message"_L1, event->fullJson());
    QCOMPARE(room->pendingEvents().size(), 1);
    const auto pendingIt = room->findPendingEvent(txID);

    QCOMPARE(EventHandler::timeString(room, pendingIt->event(), false, QLocale::ShortFormat, true),
             QLocale().toString(pendingIt->lastUpdated().toLocalTime().time(), QLocale::ShortFormat));
    QCOMPARE(EventHandler::timeString(room, pendingIt->event(), true, QLocale::ShortFormat, true),
             format.formatRelativeDate(pendingIt->lastUpdated().toLocalTime().date(), QLocale::ShortFormat));
    QCOMPARE(EventHandler::timeString(room, pendingIt->event(), false, QLocale::LongFormat, true),
             QLocale().toString(pendingIt->lastUpdated().toLocalTime().time(), QLocale::LongFormat));
    QCOMPARE(EventHandler::timeString(room, pendingIt->event(), true, QLocale::LongFormat, true),
             format.formatRelativeDate(pendingIt->lastUpdated().toLocalTime().date(), QLocale::LongFormat));

    room->discardMessage(txID);
    QCOMPARE(room->pendingEvents().size(), 0);
}

void EventHandlerTest::highlighted()
{
    QCOMPARE(EventHandler::isHighlighted(room, room->messageEvents().at(2).get()), true);
    QCOMPARE(EventHandler::isHighlighted(room, room->messageEvents().at(0).get()), false);
}

void EventHandlerTest::nullHighlighted()
{
    QTest::ignoreMessage(QtWarningMsg, "isHighlighted called with room set to nullptr.");
    QCOMPARE(EventHandler::isHighlighted(nullptr, nullptr), false);

    QTest::ignoreMessage(QtWarningMsg, "isHighlighted called with event set to nullptr.");
    QCOMPARE(EventHandler::isHighlighted(room, nullptr), false);
}

void EventHandlerTest::hidden()
{
    QCOMPARE(EventHandler::isHidden(room, room->messageEvents().at(3).get()), true);
    QCOMPARE(EventHandler::isHidden(room, room->messageEvents().at(0).get()), false);
}

void EventHandlerTest::nullHidden()
{
    QTest::ignoreMessage(QtWarningMsg, "isHidden called with room set to nullptr.");
    QCOMPARE(EventHandler::isHidden(nullptr, nullptr), false);

    QTest::ignoreMessage(QtWarningMsg, "isHidden called with event set to nullptr.");
    QCOMPARE(EventHandler::isHidden(room, nullptr), false);
}

void EventHandlerTest::body()
{
    const auto event = room->messageEvents().at(0).get();

    QCOMPARE(EventHandler::richBody(room, event), u"<b>This is an example<br>text message</b>"_s);
    QCOMPARE(EventHandler::richBody(room, event, true), u"<b>This is an example text message</b>"_s);
    QCOMPARE(EventHandler::plainBody(room, event), u"This is an example\ntext message"_s);
    QCOMPARE(EventHandler::plainBody(room, event, true), u"This is an example text message"_s);
}

void EventHandlerTest::nullBody()
{
    QTest::ignoreMessage(QtWarningMsg, "richBody called with room set to nullptr.");
    QCOMPARE(EventHandler::richBody(nullptr, nullptr), QString());

    QTest::ignoreMessage(QtWarningMsg, "richBody called with event set to nullptr.");
    QCOMPARE(EventHandler::richBody(room, nullptr), QString());

    QTest::ignoreMessage(QtWarningMsg, "plainBody called with room set to nullptr.");
    QCOMPARE(EventHandler::plainBody(nullptr, nullptr), QString());

    QTest::ignoreMessage(QtWarningMsg, "plainBody called with event set to nullptr.");
    QCOMPARE(EventHandler::plainBody(room, nullptr), QString());
}

void EventHandlerTest::genericBody_data()
{
    QTest::addColumn<int>("eventNum");
    QTest::addColumn<QString>("output");

    QTest::newRow("message") << 0 << u"<a href=\"https://matrix.to/#/@example:example.org\">after</a> sent a message"_s;
    QTest::newRow("member") << 1 << u"<a href=\"https://matrix.to/#/@example:example.org\">after</a> changed their display name and updated their avatar"_s;
    QTest::newRow("message 2") << 2 << u"<a href=\"https://matrix.to/#/@example:example.org\">after</a> sent a message"_s;
    QTest::newRow("reaction") << 3 << u"Unknown event"_s;
    QTest::newRow("video") << 4 << u"<a href=\"https://matrix.to/#/@example:example.org\">after</a> sent a message"_s;
}

void EventHandlerTest::genericBody()
{
    QFETCH(int, eventNum);
    QFETCH(QString, output);

    QCOMPARE(EventHandler::genericBody(room, room->messageEvents().at(eventNum).get()), output);
}

void EventHandlerTest::nullGenericBody()
{
    QTest::ignoreMessage(QtWarningMsg, "genericBody called with room set to nullptr.");
    QCOMPARE(EventHandler::genericBody(nullptr, nullptr), QString());

    QTest::ignoreMessage(QtWarningMsg, "genericBody called with event set to nullptr.");
    QCOMPARE(EventHandler::genericBody(room, nullptr), QString());
}

void EventHandlerTest::markdownBody()
{
    QCOMPARE(EventHandler::markdownBody(room->messageEvents().at(0).get()), u"This is an example\ntext message"_s);
}

void EventHandlerTest::markdownBodyReply()
{
    QCOMPARE(EventHandler::markdownBody(room->messageEvents().at(5).get()), u"reply"_s);
}

void EventHandlerTest::subtitle()
{
    QCOMPARE(EventHandler::subtitleText(room, room->messageEvents().at(0).get()), u"after: This is an example text message"_s);
    QCOMPARE(EventHandler::subtitleText(room, room->messageEvents().at(2).get()),
             u"after: This is a highlight @bob:kde.org and this is a link https://kde.org"_s);
}

void EventHandlerTest::nullSubtitle()
{
    QTest::ignoreMessage(QtWarningMsg, "subtitleText called with room set to nullptr.");
    QCOMPARE(EventHandler::subtitleText(nullptr, nullptr), QString());

    QTest::ignoreMessage(QtWarningMsg, "subtitleText called with event set to nullptr.");
    QCOMPARE(EventHandler::subtitleText(room, nullptr), QString());
}

void EventHandlerTest::mediaInfo()
{
    auto event = room->messageEvents().at(4).get();
    auto mediaInfo = EventHandler::mediaInfo(room, event);
    auto thumbnailInfo = mediaInfo["tempInfo"_L1].toMap();

    QCOMPARE(mediaInfo["source"_L1], room->makeMediaUrl(event->id(), QUrl("mxc://kde.org/1234567"_L1)));
    QCOMPARE(mediaInfo["mimeType"_L1], u"video/mp4"_s);
    QCOMPARE(mediaInfo["mimeIcon"_L1], u"video-mp4"_s);
    QCOMPARE(mediaInfo["size"_L1], 62650636);
    QCOMPARE(mediaInfo["duration"_L1], 10);
    QCOMPARE(mediaInfo["width"_L1], 1920);
    QCOMPARE(mediaInfo["height"_L1], 1080);
    QCOMPARE(thumbnailInfo["source"_L1], room->makeMediaUrl(event->id(), QUrl("mxc://kde.org/2234567"_L1)));
    QCOMPARE(thumbnailInfo["mimeType"_L1], u"image/jpeg"_s);
    QCOMPARE(thumbnailInfo["mimeIcon"_L1], u"image-jpeg"_s);
    QCOMPARE(thumbnailInfo["size"_L1], 382249);
    QCOMPARE(thumbnailInfo["width"_L1], 800);
    QCOMPARE(thumbnailInfo["height"_L1], 450);
}

void EventHandlerTest::nullMediaInfo()
{
    QTest::ignoreMessage(QtWarningMsg, "mediaInfo called with room set to nullptr.");
    QCOMPARE(EventHandler::mediaInfo(nullptr, nullptr), QVariantMap());

    QTest::ignoreMessage(QtWarningMsg, "mediaInfo called with event set to nullptr.");
    QCOMPARE(EventHandler::mediaInfo(room, nullptr), QVariantMap());
}

void EventHandlerTest::replyAuthor()
{
    auto replyEvent = room->messageEvents().at(0).get();
    auto replyAuthor = room->member(replyEvent->senderId());
    auto eventHandlerReplyAuthor = EventHandler::replyAuthor(room, room->messageEvents().at(5).get());

    QCOMPARE(eventHandlerReplyAuthor.isLocalMember(), replyAuthor.id() == room->localMember().id());
    QCOMPARE(eventHandlerReplyAuthor.id(), replyAuthor.id());
    QCOMPARE(eventHandlerReplyAuthor.displayName(), replyAuthor.displayName());
    QCOMPARE(eventHandlerReplyAuthor.avatarUrl(), replyAuthor.avatarUrl());
    QCOMPARE(eventHandlerReplyAuthor.avatarMediaId(), replyAuthor.avatarMediaId());
    QCOMPARE(eventHandlerReplyAuthor.color(), replyAuthor.color());

    QCOMPARE(EventHandler::replyAuthor(room, room->messageEvents().at(0).get()), RoomMember());
}

void EventHandlerTest::nullReplyAuthor()
{
    QTest::ignoreMessage(QtWarningMsg, "replyAuthor called with room set to nullptr.");
    QCOMPARE(EventHandler::replyAuthor(nullptr, nullptr), RoomMember());

    QTest::ignoreMessage(QtWarningMsg, "replyAuthor called with event set to nullptr. Returning empty user.");
    QCOMPARE(EventHandler::replyAuthor(room, nullptr), RoomMember());
}

void EventHandlerTest::location()
{
    QCOMPARE(EventHandler::latitude(room->messageEvents().at(7).get()), u"51.7035"_s.toFloat());
    QCOMPARE(EventHandler::longitude(room->messageEvents().at(7).get()), u"-1.14394"_s.toFloat());
    QCOMPARE(EventHandler::locationAssetType(room->messageEvents().at(7).get()), u"m.pin"_s);
}

void EventHandlerTest::nullLocation()
{
    QTest::ignoreMessage(QtWarningMsg, "latitude called with event set to nullptr.");
    QCOMPARE(EventHandler::latitude(nullptr), -100.0);

    QTest::ignoreMessage(QtWarningMsg, "longitude called with event set to nullptr.");
    QCOMPARE(EventHandler::longitude(nullptr), -200.0);

    QTest::ignoreMessage(QtWarningMsg, "locationAssetType called with event set to nullptr.");
    QCOMPARE(EventHandler::locationAssetType(nullptr), QString());
}

QTEST_MAIN(EventHandlerTest)
#include "eventhandlertest.moc"
