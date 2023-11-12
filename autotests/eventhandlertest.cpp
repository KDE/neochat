// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>

#include "eventhandler.h"

#include <KFormat>

#include <Quotient/connection.h>
#include <Quotient/quotient_common.h>
#include <Quotient/syncdata.h>

#include "enums/delegatetype.h"
#include "linkpreviewer.h"
#include "models/reactionmodel.h"
#include "neochatroom.h"
#include "utils.h"

using namespace Quotient;

class TestRoom : public NeoChatRoom
{
public:
    using NeoChatRoom::NeoChatRoom;

    void update(SyncRoomData &&data, bool fromCache = false)
    {
        Room::updateData(std::move(data), fromCache);
    }
};

class EventHandlerTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestRoom *room = nullptr;
    EventHandler eventHandler;
    EventHandler emptyHandler;
    EventHandler noEventHandler;

private Q_SLOTS:
    void initTestCase();

    void nullSetEvent();
    void eventId();
    void nullEventId();
    void delegateType_data();
    void delegateType();
    void nullDelegateType();
    void author();
    void nullAuthor();
    void authorDisplayName();
    void nullAuthorDisplayName();
    void singleLineSidplayName();
    void nullSingleLineDisplayName();
    void time();
    void nullTime();
    void timeString();
    void nullTimeString();
    void highlighted();
    void nullHighlighted();
    void hidden();
    void nullHidden();
    void body();
    void nullBody();
    void genericBody_data();
    void genericBody();
    void nullGenericBody();
    void mediaInfo();
    void nullMediaInfo();
    void linkPreviewer();
    void nullLinkPreviewer();
    void reactions();
    void nullReactions();
    void hasReply();
    void nullHasReply();
    void replyId();
    void nullReplyId();
    void replyDelegateType();
    void nullReplyDelegateType();
    void replyAuthor();
    void nullReplyAuthor();
    void replyBody();
    void nullReplyBody();
    void replyMediaInfo();
    void nullReplyMediaInfo();
    void thread();
    void nullThread();
    void location();
    void nullLocation();
    void readMarkers();
    void nullReadMarkers();

    void cleanup();
};

void EventHandlerTest::initTestCase()
{
    connection = Connection::makeMockConnection(QStringLiteral("@bob:kde.org"));
    room = new TestRoom(connection, QStringLiteral("#myroom:kde.org"), JoinState::Join);

    QFile testEventHandlerSyncFile;
    testEventHandlerSyncFile.setFileName(QLatin1String(DATA_DIR) + u'/' + QLatin1String("test-eventhandler-sync.json"));
    testEventHandlerSyncFile.open(QIODevice::ReadOnly);
    const auto testEventHandlerSyncJson = QJsonDocument::fromJson(testEventHandlerSyncFile.readAll());
    SyncRoomData roomData(QStringLiteral("@bob:kde.org"), JoinState::Join, testEventHandlerSyncJson.object());
    room->update(std::move(roomData));

    eventHandler.setRoom(room);
    noEventHandler.setRoom(room);
}

void EventHandlerTest::nullSetEvent()
{
    QTest::ignoreMessage(QtWarningMsg, "cannot setEvent when m_room is set to nullptr.");
    emptyHandler.setEvent(room->messageEvents().at(0).get());
}

void EventHandlerTest::eventId()
{
    eventHandler.setEvent(room->messageEvents().at(0).get());

    QCOMPARE(eventHandler.getId(), QStringLiteral("$153456789:example.org"));
}

void EventHandlerTest::nullEventId()
{
    QTest::ignoreMessage(QtWarningMsg, "getId called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getId(), QString());
}

void EventHandlerTest::delegateType_data()
{
    QTest::addColumn<int>("eventNum");
    QTest::addColumn<DelegateType::Type>("delegateType");

    QTest::newRow("message") << 0 << DelegateType::Message;
    QTest::newRow("state") << 1 << DelegateType::State;
    QTest::newRow("message 2") << 2 << DelegateType::Message;
    QTest::newRow("reaction") << 3 << DelegateType::Other;
    QTest::newRow("video") << 4 << DelegateType::Video;
    QTest::newRow("location") << 7 << DelegateType::Location;
}

void EventHandlerTest::delegateType()
{
    QFETCH(int, eventNum);
    QFETCH(DelegateType::Type, delegateType);

    eventHandler.setEvent(room->messageEvents().at(eventNum).get());

    QCOMPARE(eventHandler.getDelegateType(), delegateType);
}

void EventHandlerTest::nullDelegateType()
{
    QTest::ignoreMessage(QtWarningMsg, "getDelegateType called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getDelegateType(), DelegateType::Other);
}

void EventHandlerTest::author()
{
    auto event = room->messageEvents().at(0).get();
    auto author = room->user(event->senderId());
    eventHandler.setEvent(event);

    auto eventHandlerAuthor = eventHandler.getAuthor();

    QCOMPARE(eventHandlerAuthor["isLocalUser"_ls], author->id() == room->localUser()->id());
    QCOMPARE(eventHandlerAuthor["id"_ls], author->id());
    QCOMPARE(eventHandlerAuthor["displayName"_ls], author->displayname(room));
    QCOMPARE(eventHandlerAuthor["avatarSource"_ls], room->avatarForMember(author));
    QCOMPARE(eventHandlerAuthor["avatarMediaId"_ls], author->avatarMediaId(room));
    QCOMPARE(eventHandlerAuthor["color"_ls], Utils::getUserColor(author->hueF()));
    QCOMPARE(eventHandlerAuthor["object"_ls], QVariant::fromValue(author));
}

void EventHandlerTest::nullAuthor()
{
    QTest::ignoreMessage(QtWarningMsg, "getAuthor called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.getAuthor(), QVariantMap());

    QTest::ignoreMessage(QtWarningMsg, "getAuthor called with m_event set to nullptr. Returning empty user.");
    QCOMPARE(noEventHandler.getAuthor(), room->getUser(nullptr));
}

void EventHandlerTest::authorDisplayName()
{
    auto event = room->messageEvents().at(1).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getAuthorDisplayName(), QStringLiteral("before"));
}

void EventHandlerTest::nullAuthorDisplayName()
{
    QTest::ignoreMessage(QtWarningMsg, "getAuthorDisplayName called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.getAuthorDisplayName(), QString());

    QTest::ignoreMessage(QtWarningMsg, "getAuthorDisplayName called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getAuthorDisplayName(), QString());
}

void EventHandlerTest::singleLineSidplayName()
{
    auto event = room->messageEvents().at(11).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.singleLineAuthorDisplayname(), QStringLiteral("Look at me I put newlines in my display name"));
}

void EventHandlerTest::nullSingleLineDisplayName()
{
    QTest::ignoreMessage(QtWarningMsg, "getAuthorDisplayName called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.singleLineAuthorDisplayname(), QString());

    QTest::ignoreMessage(QtWarningMsg, "getAuthorDisplayName called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.singleLineAuthorDisplayname(), QString());
}

void EventHandlerTest::time()
{
    auto event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getTime(), QDateTime::fromMSecsSinceEpoch(1432735824654, Qt::UTC));
    QCOMPARE(eventHandler.getTime(true, QDateTime::fromMSecsSinceEpoch(1234, Qt::UTC)), QDateTime::fromMSecsSinceEpoch(1234, Qt::UTC));
}

void EventHandlerTest::nullTime()
{
    QTest::ignoreMessage(QtWarningMsg, "getTime called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getTime(), QDateTime());

    eventHandler.setEvent(room->messageEvents().at(0).get());
    QTest::ignoreMessage(QtWarningMsg, "a value must be provided for lastUpdated for a pending event.");
    QCOMPARE(eventHandler.getTime(true), QDateTime());
}

void EventHandlerTest::timeString()
{
    auto event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    KFormat format;

    QCOMPARE(eventHandler.getTimeString(false),
             QLocale().toString(QDateTime::fromMSecsSinceEpoch(1432735824654, Qt::UTC).toLocalTime().time(), QLocale::ShortFormat));
    QCOMPARE(eventHandler.getTimeString(true),
             format.formatRelativeDate(QDateTime::fromMSecsSinceEpoch(1432735824654, Qt::UTC).toLocalTime().date(), QLocale::ShortFormat));
    QCOMPARE(eventHandler.getTimeString(false, QLocale::ShortFormat, true, QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC)),
             QLocale().toString(QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC).toLocalTime().time(), QLocale::ShortFormat));
    QCOMPARE(eventHandler.getTimeString(true, QLocale::ShortFormat, true, QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC)),
             format.formatRelativeDate(QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC).toLocalTime().date(), QLocale::ShortFormat));
    QCOMPARE(eventHandler.getTimeString(false, QLocale::LongFormat, true, QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC)),
             QLocale().toString(QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC).toLocalTime().time(), QLocale::LongFormat));
    QCOMPARE(eventHandler.getTimeString(true, QLocale::LongFormat, true, QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC)),
             format.formatRelativeDate(QDateTime::fromMSecsSinceEpoch(1690699214545, Qt::UTC).toLocalTime().date(), QLocale::LongFormat));
}

void EventHandlerTest::nullTimeString()
{
    QTest::ignoreMessage(QtWarningMsg, "getTimeString called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getTimeString(false), QString());

    eventHandler.setEvent(room->messageEvents().at(0).get());
    QTest::ignoreMessage(QtWarningMsg, "a value must be provided for lastUpdated for a pending event.");
    QCOMPARE(eventHandler.getTimeString(false, QLocale::ShortFormat, true), QString());
}

void EventHandlerTest::highlighted()
{
    auto event = room->messageEvents().at(2).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.isHighlighted(), true);

    event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.isHighlighted(), false);
}

void EventHandlerTest::nullHighlighted()
{
    QTest::ignoreMessage(QtWarningMsg, "isHighlighted called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.isHighlighted(), false);

    QTest::ignoreMessage(QtWarningMsg, "isHighlighted called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.isHighlighted(), false);
}

void EventHandlerTest::hidden()
{
    auto event = room->messageEvents().at(3).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.isHidden(), true);

    event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.isHidden(), false);
}

void EventHandlerTest::nullHidden()
{
    QTest::ignoreMessage(QtWarningMsg, "isHidden called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.isHidden(), false);

    QTest::ignoreMessage(QtWarningMsg, "isHidden called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.isHidden(), false);
}

void EventHandlerTest::body()
{
    auto event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getRichBody(), QStringLiteral("<b>This is an example<br>text message</b>"));
    QCOMPARE(eventHandler.getRichBody(true), QStringLiteral("<b>This is an example text message</b>"));
    QCOMPARE(eventHandler.getPlainBody(), QStringLiteral("This is an example\ntext message"));
    QCOMPARE(eventHandler.getPlainBody(true), QStringLiteral("This is an example text message"));
}

void EventHandlerTest::nullBody()
{
    QTest::ignoreMessage(QtWarningMsg, "getRichBody called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getRichBody(), QString());

    QTest::ignoreMessage(QtWarningMsg, "getPlainBody called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getPlainBody(), QString());
}

void EventHandlerTest::genericBody_data()
{
    QTest::addColumn<int>("eventNum");
    QTest::addColumn<QString>("output");

    QTest::newRow("message") << 0 << QStringLiteral("sent a message");
    QTest::newRow("member") << 1 << QStringLiteral("changed their display name and updated their avatar");
    QTest::newRow("message 2") << 2 << QStringLiteral("sent a message");
    QTest::newRow("reaction") << 3 << QStringLiteral("Unknown event");
    QTest::newRow("video") << 4 << QStringLiteral("sent a message");
}

void EventHandlerTest::genericBody()
{
    QFETCH(int, eventNum);
    QFETCH(QString, output);

    eventHandler.setEvent(room->messageEvents().at(eventNum).get());

    QCOMPARE(eventHandler.getGenericBody(), output);
}

void EventHandlerTest::nullGenericBody()
{
    QTest::ignoreMessage(QtWarningMsg, "getGenericBody called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getGenericBody(), QString());
}

void EventHandlerTest::mediaInfo()
{
    auto event = room->messageEvents().at(4).get();
    eventHandler.setEvent(event);

    auto mediaInfo = eventHandler.getMediaInfo();
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
    QTest::ignoreMessage(QtWarningMsg, "getMediaInfo called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.getMediaInfo(), QVariantMap());

    QTest::ignoreMessage(QtWarningMsg, "getMediaInfo called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getMediaInfo(), QVariantMap());
}

void EventHandlerTest::linkPreviewer()
{
    auto event = room->messageEvents().at(2).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getLinkPreviewer()->url(), QUrl("https://kde.org"_ls));

    event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getLinkPreviewer(), nullptr);
}

void EventHandlerTest::nullLinkPreviewer()
{
    QTest::ignoreMessage(QtWarningMsg, "getLinkPreviewer called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.getLinkPreviewer(), nullptr);

    QTest::ignoreMessage(QtWarningMsg, "getLinkPreviewer called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getLinkPreviewer(), nullptr);
}

void EventHandlerTest::reactions()
{
    auto event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getReactions()->rowCount(), 1);
}

void EventHandlerTest::nullReactions()
{
    QTest::ignoreMessage(QtWarningMsg, "getReactions called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.getReactions(), nullptr);

    QTest::ignoreMessage(QtWarningMsg, "getReactions called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getReactions(), nullptr);
}

void EventHandlerTest::hasReply()
{
    auto event = room->messageEvents().at(5).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.hasReply(), true);

    event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.hasReply(), false);
}

void EventHandlerTest::nullHasReply()
{
    QTest::ignoreMessage(QtWarningMsg, "hasReply called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.hasReply(), false);
}

void EventHandlerTest::replyId()
{
    auto event = room->messageEvents().at(5).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getReplyId(), QStringLiteral("$153456789:example.org"));

    event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getReplyId(), QStringLiteral(""));
}

void EventHandlerTest::nullReplyId()
{
    QTest::ignoreMessage(QtWarningMsg, "getReplyId called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getReplyId(), QString());
}

void EventHandlerTest::replyDelegateType()
{
    auto event = room->messageEvents().at(5).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getReplyDelegateType(), DelegateType::Message);

    event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getReplyDelegateType(), DelegateType::Other);
}

void EventHandlerTest::nullReplyDelegateType()
{
    QTest::ignoreMessage(QtWarningMsg, "getReplyDelegateType called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.getReplyDelegateType(), DelegateType::Other);

    QTest::ignoreMessage(QtWarningMsg, "getReplyDelegateType called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getReplyDelegateType(), DelegateType::Other);
}

void EventHandlerTest::replyAuthor()
{
    auto event = room->messageEvents().at(5).get();
    auto replyEvent = room->messageEvents().at(0).get();
    auto replyAuthor = room->user(replyEvent->senderId());
    eventHandler.setEvent(event);

    auto eventHandlerReplyAuthor = eventHandler.getReplyAuthor();

    QCOMPARE(eventHandlerReplyAuthor["isLocalUser"_ls], replyAuthor->id() == room->localUser()->id());
    QCOMPARE(eventHandlerReplyAuthor["id"_ls], replyAuthor->id());
    QCOMPARE(eventHandlerReplyAuthor["displayName"_ls], replyAuthor->displayname(room));
    QCOMPARE(eventHandlerReplyAuthor["avatarSource"_ls], room->avatarForMember(replyAuthor));
    QCOMPARE(eventHandlerReplyAuthor["avatarMediaId"_ls], replyAuthor->avatarMediaId(room));
    QCOMPARE(eventHandlerReplyAuthor["color"_ls], Utils::getUserColor(replyAuthor->hueF()));
    QCOMPARE(eventHandlerReplyAuthor["object"_ls], QVariant::fromValue(replyAuthor));

    event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getReplyAuthor(), room->getUser(nullptr));
}

void EventHandlerTest::nullReplyAuthor()
{
    QTest::ignoreMessage(QtWarningMsg, "getReplyAuthor called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.getReplyAuthor(), QVariantMap());

    QTest::ignoreMessage(QtWarningMsg, "getReplyAuthor called with m_event set to nullptr. Returning empty user.");
    QCOMPARE(noEventHandler.getReplyAuthor(), room->getUser(nullptr));
}

void EventHandlerTest::replyBody()
{
    auto event = room->messageEvents().at(5).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getReplyRichBody(), QStringLiteral("<b>This is an example<br>text message</b>"));
    QCOMPARE(eventHandler.getReplyRichBody(true), QStringLiteral("<b>This is an example text message</b>"));
    QCOMPARE(eventHandler.getReplyPlainBody(), QStringLiteral("This is an example\ntext message"));
    QCOMPARE(eventHandler.getReplyPlainBody(true), QStringLiteral("This is an example text message"));
}

void EventHandlerTest::nullReplyBody()
{
    QTest::ignoreMessage(QtWarningMsg, "getReplyRichBody called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getReplyRichBody(), QString());

    QTest::ignoreMessage(QtWarningMsg, "getReplyPlainBody called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getReplyPlainBody(), QString());
}

void EventHandlerTest::replyMediaInfo()
{
    auto event = room->messageEvents().at(6).get();
    auto replyEvent = room->messageEvents().at(4).get();
    eventHandler.setEvent(event);

    auto mediaInfo = eventHandler.getReplyMediaInfo();
    auto thumbnailInfo = mediaInfo["tempInfo"_ls].toMap();

    QCOMPARE(mediaInfo["source"_ls], room->makeMediaUrl(replyEvent->id(), QUrl("mxc://kde.org/1234567"_ls)));
    QCOMPARE(mediaInfo["mimeType"_ls], QStringLiteral("video/mp4"));
    QCOMPARE(mediaInfo["mimeIcon"_ls], QStringLiteral("video-mp4"));
    QCOMPARE(mediaInfo["size"_ls], 62650636);
    QCOMPARE(mediaInfo["duration"_ls], 10);
    QCOMPARE(mediaInfo["width"_ls], 1920);
    QCOMPARE(mediaInfo["height"_ls], 1080);
    QCOMPARE(thumbnailInfo["source"_ls], room->makeMediaUrl(replyEvent->id(), QUrl("mxc://kde.org/2234567"_ls)));
    QCOMPARE(thumbnailInfo["mimeType"_ls], QStringLiteral("image/jpeg"));
    QCOMPARE(thumbnailInfo["mimeIcon"_ls], QStringLiteral("image-jpeg"));
    QCOMPARE(thumbnailInfo["size"_ls], 382249);
    QCOMPARE(thumbnailInfo["width"_ls], 800);
    QCOMPARE(thumbnailInfo["height"_ls], 450);
}

void EventHandlerTest::nullReplyMediaInfo()
{
    QTest::ignoreMessage(QtWarningMsg, "getReplyMediaInfo called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.getReplyMediaInfo(), QVariantMap());

    QTest::ignoreMessage(QtWarningMsg, "getReplyMediaInfo called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getReplyMediaInfo(), QVariantMap());
}

void EventHandlerTest::thread()
{
    auto event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.isThreaded(), false);
    QCOMPARE(eventHandler.threadRoot(), QString());

    event = room->messageEvents().at(9).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.isThreaded(), true);
    QCOMPARE(eventHandler.threadRoot(), QStringLiteral("$threadroot:example.org"));
    QCOMPARE(eventHandler.getReplyId(), QStringLiteral("$threadroot:example.org"));

    event = room->messageEvents().at(10).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.isThreaded(), true);
    QCOMPARE(eventHandler.threadRoot(), QStringLiteral("$threadroot:example.org"));
    QCOMPARE(eventHandler.getReplyId(), QStringLiteral("$threadmessage1:example.org"));
}

void EventHandlerTest::nullThread()
{
    QTest::ignoreMessage(QtWarningMsg, "isThreaded called with m_event set to nullptr.");
    QCOMPARE(emptyHandler.isThreaded(), false);

    QTest::ignoreMessage(QtWarningMsg, "threadRoot called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.threadRoot(), QString());
}

void EventHandlerTest::location()
{
    auto event = room->messageEvents().at(7).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getLatitude(), QStringLiteral("51.7035").toFloat());
    QCOMPARE(eventHandler.getLongitude(), QStringLiteral("-1.14394").toFloat());
    QCOMPARE(eventHandler.getLocationAssetType(), QStringLiteral("m.pin"));
}

void EventHandlerTest::nullLocation()
{
    QTest::ignoreMessage(QtWarningMsg, "getLatitude called with m_event set to nullptr.");
    QCOMPARE(emptyHandler.getLatitude(), -100.0);

    QTest::ignoreMessage(QtWarningMsg, "getLongitude called with m_event set to nullptr.");
    QCOMPARE(emptyHandler.getLongitude(), -200.0);

    QTest::ignoreMessage(QtWarningMsg, "getLocationAssetType called with m_event set to nullptr.");
    QCOMPARE(emptyHandler.getLocationAssetType(), QString());
}

void EventHandlerTest::readMarkers()
{
    auto event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.hasReadMarkers(), true);

    auto readMarkers = eventHandler.getReadMarkers();

    QCOMPARE(readMarkers.size(), 1);
    QCOMPARE(readMarkers[0].toMap()["id"_ls], QStringLiteral("@alice:matrix.org"));

    QCOMPARE(eventHandler.getNumberExcessReadMarkers(), QString());
    QCOMPARE(eventHandler.getReadMarkersString(), QStringLiteral("1 user: @alice:matrix.org"));

    event = room->messageEvents().at(2).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.hasReadMarkers(), true);

    readMarkers = eventHandler.getReadMarkers();

    QCOMPARE(readMarkers.size(), 5);

    QCOMPARE(eventHandler.getNumberExcessReadMarkers(), QStringLiteral("+ 1"));
    // There are no guarantees on the order of the users it will be different every time so don't match the whole string.
    QCOMPARE(eventHandler.getReadMarkersString().startsWith(QStringLiteral("6 users:")), true);
}

void EventHandlerTest::nullReadMarkers()
{
    QTest::ignoreMessage(QtWarningMsg, "hasReadMarkers called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.hasReadMarkers(), false);

    QTest::ignoreMessage(QtWarningMsg, "getReadMarkers called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.getReadMarkers(), QVariantList());

    QTest::ignoreMessage(QtWarningMsg, "getNumberExcessReadMarkers called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.getNumberExcessReadMarkers(), QString());

    QTest::ignoreMessage(QtWarningMsg, "getReadMarkersString called with m_room set to nullptr.");
    QCOMPARE(emptyHandler.getReadMarkersString(), QString());

    QTest::ignoreMessage(QtWarningMsg, "hasReadMarkers called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.hasReadMarkers(), false);

    QTest::ignoreMessage(QtWarningMsg, "getReadMarkers called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getReadMarkers(), QVariantList());

    QTest::ignoreMessage(QtWarningMsg, "getNumberExcessReadMarkers called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getNumberExcessReadMarkers(), QString());

    QTest::ignoreMessage(QtWarningMsg, "getReadMarkersString called with m_event set to nullptr.");
    QCOMPARE(noEventHandler.getReadMarkersString(), QString());
}

void EventHandlerTest::cleanup()
{
    eventHandler.setEvent(nullptr);
}

QTEST_MAIN(EventHandlerTest)
#include "eventhandlertest.moc"
