// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>

#include "eventhandler.h"

#include <KFormat>

#include <Quotient/connection.h>
#include <Quotient/quotient_common.h>
#include <Quotient/syncdata.h>

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

    void eventId();
    void nullEventId();
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
    void hasReply();
    void nullHasReply();
    void replyId();
    void nullReplyId();
    void replyAuthor();
    void nullReplyAuthor();
    void thread();
    void nullThread();
    void location();
    void nullLocation();
};

void EventHandlerTest::initTestCase()
{
    connection = Connection::makeMockConnection(QStringLiteral("@bob:kde.org"));
    room = new TestUtils::TestRoom(connection, QStringLiteral("#myroom:kde.org"), QLatin1String("test-eventhandler-sync.json"));
}

void EventHandlerTest::eventId()
{
    QCOMPARE(EventHandler::id(room->messageEvents().at(0).get()), QStringLiteral("$153456789:example.org"));
}

void EventHandlerTest::nullEventId()
{
    QTest::ignoreMessage(QtWarningMsg, "id called with event set to nullptr.");
    QCOMPARE(EventHandler::id(nullptr), QString());
}

void EventHandlerTest::authorDisplayName()
{
    QCOMPARE(EventHandler::authorDisplayName(room, room->messageEvents().at(1).get()), QStringLiteral("before"));
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
    QCOMPARE(EventHandler::singleLineAuthorDisplayname(room, room->messageEvents().at(11).get()),
             QStringLiteral("Look at me I put newlines in my display name"));
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

    QCOMPARE(EventHandler::time(event), QDateTime::fromMSecsSinceEpoch(1432735824654, Qt::UTC));
    QCOMPARE(EventHandler::time(event, true, QDateTime::fromMSecsSinceEpoch(1234, Qt::UTC)), QDateTime::fromMSecsSinceEpoch(1234, Qt::UTC));
}

void EventHandlerTest::nullTime()
{
    QTest::ignoreMessage(QtWarningMsg, "time called with event set to nullptr.");
    QCOMPARE(EventHandler::time(nullptr), QDateTime());

    QTest::ignoreMessage(QtWarningMsg, "a value must be provided for lastUpdated for a pending event.");
    QCOMPARE(EventHandler::time(room->messageEvents().at(0).get(), true), QDateTime());
}

void EventHandlerTest::timeString()
{
    const auto event = room->messageEvents().at(0).get();

    KFormat format;

    QCOMPARE(EventHandler::timeString(event, false),
             QLocale().toString(QDateTime::fromMSecsSinceEpoch(1432735824654, Qt::UTC).toLocalTime().time(), QLocale::ShortFormat));
    QCOMPARE(EventHandler::timeString(event, true),
             format.formatRelativeDate(QDateTime::fromMSecsSinceEpoch(1432735824654, Qt::UTC).toLocalTime().date(), QLocale::ShortFormat));
    QCOMPARE(EventHandler::timeString(event, false, QLocale::ShortFormat, true, QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC)),
             QLocale().toString(QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC).toLocalTime().time(), QLocale::ShortFormat));
    QCOMPARE(EventHandler::timeString(event, true, QLocale::ShortFormat, true, QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC)),
             format.formatRelativeDate(QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC).toLocalTime().date(), QLocale::ShortFormat));
    QCOMPARE(EventHandler::timeString(event, false, QLocale::LongFormat, true, QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC)),
             QLocale().toString(QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC).toLocalTime().time(), QLocale::LongFormat));
    QCOMPARE(EventHandler::timeString(event, true, QLocale::LongFormat, true, QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC)),
             format.formatRelativeDate(QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC).toLocalTime().date(), QLocale::LongFormat));
    QCOMPARE(EventHandler::timeString(event, QStringLiteral("hh:mm")),
             QDateTime::fromMSecsSinceEpoch(1432735824654, Qt::UTC).toString(QStringLiteral("hh:mm")));
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

    QCOMPARE(EventHandler::richBody(room, event), QStringLiteral("<b>This is an example<br>text message</b>"));
    QCOMPARE(EventHandler::richBody(room, event, true), QStringLiteral("<b>This is an example text message</b>"));
    QCOMPARE(EventHandler::plainBody(room, event), QStringLiteral("This is an example\ntext message"));
    QCOMPARE(EventHandler::plainBody(room, event, true), QStringLiteral("This is an example text message"));
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

    QTest::newRow("message") << 0 << QStringLiteral("<a href=\"https://matrix.to/#/@example:example.org\">after</a> sent a message");
    QTest::newRow("member") << 1
                            << QStringLiteral(
                                   "<a href=\"https://matrix.to/#/@example:example.org\">after</a> changed their display name and updated their avatar");
    QTest::newRow("message 2") << 2 << QStringLiteral("<a href=\"https://matrix.to/#/@example:example.org\">after</a> sent a message");
    QTest::newRow("reaction") << 3 << QStringLiteral("Unknown event");
    QTest::newRow("video") << 4 << QStringLiteral("<a href=\"https://matrix.to/#/@example:example.org\">after</a> sent a message");
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
    QCOMPARE(EventHandler::markdownBody(room->messageEvents().at(0).get()), QStringLiteral("This is an example\ntext message"));
}

void EventHandlerTest::markdownBodyReply()
{
    QCOMPARE(EventHandler::markdownBody(room->messageEvents().at(5).get()), QStringLiteral("reply"));
}

void EventHandlerTest::subtitle()
{
    QCOMPARE(EventHandler::subtitleText(room, room->messageEvents().at(0).get()), QStringLiteral("after: This is an example text message"));
    QCOMPARE(EventHandler::subtitleText(room, room->messageEvents().at(2).get()),
             QStringLiteral("after: This is a highlight @bob:kde.org and this is a link https://kde.org"));
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
    auto thumbnailInfo = mediaInfo["tempInfo"_ls].toMap();

    QCOMPARE(mediaInfo["source"_ls], room->makeMediaUrl(event->id(), QUrl("mxc://kde.org/1234567"_ls)));
    QCOMPARE(mediaInfo["mimeType"_ls], QStringLiteral("video/mp4"));
    QCOMPARE(mediaInfo["mimeIcon"_ls], QStringLiteral("video-mp4"));
    QCOMPARE(mediaInfo["size"_ls], 62650636);
    QCOMPARE(mediaInfo["duration"_ls], 10);
    QCOMPARE(mediaInfo["width"_ls], 1920);
    QCOMPARE(mediaInfo["height"_ls], 1080);
    QCOMPARE(thumbnailInfo["source"_ls], room->makeMediaUrl(event->id(), QUrl("mxc://kde.org/2234567"_ls)));
    QCOMPARE(thumbnailInfo["mimeType"_ls], QStringLiteral("image/jpeg"));
    QCOMPARE(thumbnailInfo["mimeIcon"_ls], QStringLiteral("image-jpeg"));
    QCOMPARE(thumbnailInfo["size"_ls], 382249);
    QCOMPARE(thumbnailInfo["width"_ls], 800);
    QCOMPARE(thumbnailInfo["height"_ls], 450);
}

void EventHandlerTest::nullMediaInfo()
{
    QTest::ignoreMessage(QtWarningMsg, "mediaInfo called with room set to nullptr.");
    QCOMPARE(EventHandler::mediaInfo(nullptr, nullptr), QVariantMap());

    QTest::ignoreMessage(QtWarningMsg, "mediaInfo called with event set to nullptr.");
    QCOMPARE(EventHandler::mediaInfo(room, nullptr), QVariantMap());
}

void EventHandlerTest::hasReply()
{
    QCOMPARE(EventHandler::hasReply(room->messageEvents().at(5).get()), true);
    QCOMPARE(EventHandler::hasReply(room->messageEvents().at(0).get()), false);
}

void EventHandlerTest::nullHasReply()
{
    QTest::ignoreMessage(QtWarningMsg, "hasReply called with event set to nullptr.");
    QCOMPARE(EventHandler::hasReply(nullptr), false);
}

void EventHandlerTest::replyId()
{
    QCOMPARE(EventHandler::replyId(room->messageEvents().at(5).get()), QStringLiteral("$153456789:example.org"));
    QCOMPARE(EventHandler::replyId(room->messageEvents().at(0).get()), QStringLiteral(""));
}

void EventHandlerTest::nullReplyId()
{
    QTest::ignoreMessage(QtWarningMsg, "replyId called with event set to nullptr.");
    QCOMPARE(EventHandler::replyId(nullptr), QString());
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

void EventHandlerTest::thread()
{
    QCOMPARE(EventHandler::isThreaded(room->messageEvents().at(0).get()), false);
    QCOMPARE(EventHandler::threadRoot(room->messageEvents().at(0).get()), QString());

    QCOMPARE(EventHandler::isThreaded(room->messageEvents().at(9).get()), true);
    QCOMPARE(EventHandler::threadRoot(room->messageEvents().at(9).get()), QStringLiteral("$threadroot:example.org"));
    QCOMPARE(EventHandler::replyId(room->messageEvents().at(9).get()), QStringLiteral("$threadroot:example.org"));

    QCOMPARE(EventHandler::isThreaded(room->messageEvents().at(10).get()), true);
    QCOMPARE(EventHandler::threadRoot(room->messageEvents().at(10).get()), QStringLiteral("$threadroot:example.org"));
    QCOMPARE(EventHandler::replyId(room->messageEvents().at(10).get()), QStringLiteral("$threadmessage1:example.org"));
}

void EventHandlerTest::nullThread()
{
    QTest::ignoreMessage(QtWarningMsg, "isThreaded called with event set to nullptr.");
    QCOMPARE(EventHandler::isThreaded(nullptr), false);

    QTest::ignoreMessage(QtWarningMsg, "threadRoot called with event set to nullptr.");
    QCOMPARE(EventHandler::threadRoot(nullptr), QString());
}

void EventHandlerTest::location()
{
    QCOMPARE(EventHandler::latitude(room->messageEvents().at(7).get()), QStringLiteral("51.7035").toFloat());
    QCOMPARE(EventHandler::longitude(room->messageEvents().at(7).get()), QStringLiteral("-1.14394").toFloat());
    QCOMPARE(EventHandler::locationAssetType(room->messageEvents().at(7).get()), QStringLiteral("m.pin"));
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
