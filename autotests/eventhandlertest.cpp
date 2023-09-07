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

private Q_SLOTS:
    void initTestCase();

    void eventId();
    void delegateType_data();
    void delegateType();
    void author();
    void authorDisplayName();
    void time();
    void timeString();
    void highlighted();
    void hidden();
    void body();
    void genericBody_data();
    void genericBody();
    void mediaInfo();
    void linkPreviewer();
    void reactions();
    void hasReply();
    void replyId();
    void replyDelegateType();
    void replyAuthor();
    void replyBody();
    void replyMediaInfo();
    void location();
    void readMarkers();
};

void EventHandlerTest::initTestCase()
{
    connection = Connection::makeMockConnection(QStringLiteral("@bob:kde.org"));
    room = new TestRoom(connection, QStringLiteral("#myroom:kde.org"), JoinState::Join);

    const auto json = QJsonDocument::fromJson(R"EVENT({
  "account_data": {
    "events": [
      {
        "content": {
          "tags": {
            "u.work": {
              "order": 0.9
            }
          }
        },
        "type": "m.tag"
      },
      {
        "content": {
          "custom_config_key": "custom_config_value"
        },
        "type": "org.example.custom.room.config"
      }
    ]
  },
  "ephemeral": {
    "events": [
      {
        "content": {
          "user_ids": [
            "@alice:matrix.org",
            "@bob:example.com"
          ]
        },
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
        "type": "m.typing"
      },
      {
        "content": {
          "$153456789:example.org": {
            "m.read": {
              "@alice:matrix.org": {
                "ts": 1436451550453
              }
            }
          }
        },
        "type": "m.receipt"
      },
      {
        "content": {
          "$1532735824654:example.org": {
            "m.read": {
              "@bob:example.com": {
                "ts": 1436451550453
              }
            }
          }
        },
        "type": "m.receipt"
      },
      {
        "content": {
          "$1532735824654:example.org": {
            "m.read": {
              "@tim:example.com": {
                "ts": 1436451550454
              }
            }
          }
        },
        "type": "m.receipt"
      },
      {
        "content": {
          "$1532735824654:example.org": {
            "m.read": {
              "@jeff:example.com": {
                "ts": 1436451550455
              }
            }
          }
        },
        "type": "m.receipt"
      },
      {
        "content": {
          "$1532735824654:example.org": {
            "m.read": {
              "@tina:example.com": {
                "ts": 1436451550456
              }
            }
          }
        },
        "type": "m.receipt"
      },
      {
        "content": {
          "$1532735824654:example.org": {
            "m.read": {
              "@sally:example.com": {
                "ts": 1436451550457
              }
            }
          }
        },
        "type": "m.receipt"
      },
      {
        "content": {
          "$1532735824654:example.org": {
            "m.read": {
              "@fred:example.com": {
                "ts": 1436451550458
              }
            }
          }
        },
        "type": "m.receipt"
      }
    ]
  },
  "state": {
    "events": [
      {
        "content": {
          "avatar_url": "mxc://example.org/SEsfnsuifSDFSSEF",
          "displayname": "Alice Margatroid",
          "membership": "join",
          "reason": "Looking for support"
        },
        "event_id": "$143273582443PhrSn:example.org",
        "origin_server_ts": 1432735824653,
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
        "sender": "@example:example.org",
        "state_key": "@alice:example.org",
        "type": "m.room.member",
        "unsigned": {
          "age": 1234
        }
      }
    ]
  },
  "summary": {
    "m.heroes": [
      "@alice:example.com",
      "@bob:example.com"
    ],
    "m.invited_member_count": 0,
    "m.joined_member_count": 2
  },
  "timeline": {
    "events": [
      {
        "content": {
          "body": "This is an example\ntext message",
          "format": "org.matrix.custom.html",
          "formatted_body": "<b>This is an example<br>text message</b>",
          "msgtype": "m.text"
        },
        "event_id": "$153456789:example.org",
        "origin_server_ts": 1432735824654,
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
        "sender": "@example:example.org",
        "type": "m.room.message",
        "unsigned": {
          "age": 1232
        }
      },
      {
        "content": {
          "avatar_url": "mxc://kde.org/123456",
          "displayname": "after",
          "membership": "join"
        },
        "origin_server_ts": 1690651134736,
        "sender": "@example:example.org",
        "state_key": "@example:example.org",
        "type": "m.room.member",
        "unsigned": {
          "replaces_state": "$1234567890:example.org",
          "prev_content": {
            "avatar_url": "mxc://kde.org/12345",
            "displayname": "before",
            "membership": "join"
          },
          "prev_sender": "@example:example.orgg",
          "age": 1234
        },
        "event_id": "$143273583553PhrSn:example.org",
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org"
      },
      {
        "content": {
          "body": "This is a highlight @bob:kde.org and this is a link https://kde.org",
          "format": "org.matrix.custom.html",
          "msgtype": "m.text"
        },
        "event_id": "$1532735824654:example.org",
        "origin_server_ts": 1532735824654,
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
        "sender": "@example:example.org",
        "type": "m.room.message",
        "unsigned": {
          "age": 1233
        }
      },
      {
        "content": {
            "m.relates_to": {
            "event_id": "$153456789:example.org",
            "key": "👍",
            "rel_type": "m.annotation"
            }
        },
        "origin_server_ts": 1690322545182,
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
        "sender": "@alice:matrix.org",
        "type": "m.reaction",
        "unsigned": {
            "age": 390159120
        },
        "event_id": "$163456789:example.org",
        "age": 390159120
      },
      {
        "age": 4926305285,
        "content": {
            "body": "video caption",
            "filename": "video.mp4",
            "info": {
                "duration": 10,
                "h": 1080,
                "mimetype": "video/mp4",
                "size": 62650636,
                "w": 1920,
                "thumbnail_info": {
                    "h": 450,
                    "mimetype": "image/jpeg",
                    "size": 382249,
                    "w": 800
                },
                "thumbnail_url": "mxc://kde.org/2234567"
            },
            "msgtype": "m.video",
            "url": "mxc://kde.org/1234567"
        },
        "event_id": "$263456789:example.org",
        "origin_server_ts": 1685793783330,
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
        "sender": "@example:example.org",
        "type": "m.room.message",
        "unsigned": {
            "age": 4926305285
        },
        "user_id": "@example:example.org"
      },
      {
        "content": {
            "body": "> <@example:example.org> This is an example\ntext message\n\nreply",
            "format": "org.matrix.custom.html",
            "formatted_body": "<mx-reply><blockquote><a href=\"https://matrix.to/#/!jEsUZKDJdhlrceRyVU:example.org/$153456789:example.org?via=kde.org&via=matrix.org\">In reply to</a> <a href=\"https://matrix.to/#/@example:example.org\">@example:example.org</a><br><b>This is an example<br>text message</b></blockquote></mx-reply>reply",
            "m.relates_to": {
                "m.in_reply_to": {
                    "event_id": "$153456789:example.org"
                }
            },
            "msgtype": "m.text"
        },
        "origin_server_ts": 1690725965572,
        "sender": "@alice:matrix.org",
        "type": "m.room.message",
        "unsigned": {
            "age": 98
        },
        "event_id": "$154456789:example.org",
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org"
      },
      {
        "content": {
            "body": "> <@example:example.org> video caption\n\nreply",
            "m.relates_to": {
                "m.in_reply_to": {
                    "event_id": "$263456789:example.org"
                }
            },
            "msgtype": "m.text"
        },
        "origin_server_ts": 1690725965573,
        "sender": "@alice:matrix.org",
        "type": "m.room.message",
        "unsigned": {
            "age": 98
        },
        "event_id": "$154456799:example.org",
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org"
      },
      {
        "age": 96845207,
        "content": {
            "body": "Lat: 51.7035, Lon: -1.14394",
            "geo_uri": "geo:51.7035,-1.14394",
            "msgtype": "m.location",
            "org.matrix.msc1767.text": "Lat: 51.7035, Lon: -1.14394",
            "org.matrix.msc3488.asset": {
                "type": "m.pin"
            },
            "org.matrix.msc3488.location": {
                "uri": "geo:51.7035,-1.14394"
            }
        },
        "event_id": "$1544567999:example.org",
        "origin_server_ts": 1690821582876,
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
        "sender": "@example:example.org",
        "type": "m.room.message",
        "unsigned": {
            "age": 96845207
        }
      }
    ],
    "limited": true,
    "prev_batch": "t34-23535_0_0"
  }
})EVENT");
    SyncRoomData roomData(QStringLiteral("@bob:kde.org"), JoinState::Join, json.object());
    room->update(std::move(roomData));

    eventHandler.setRoom(room);
}

void EventHandlerTest::eventId()
{
    eventHandler.setEvent(room->messageEvents().at(0).get());

    QCOMPARE(eventHandler.getId(), QStringLiteral("$153456789:example.org"));
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

void EventHandlerTest::authorDisplayName()
{
    auto event = room->messageEvents().at(1).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getAuthorDisplayName(), QStringLiteral("before"));
}

void EventHandlerTest::time()
{
    auto event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getTime(), QDateTime::fromMSecsSinceEpoch(1432735824654, Qt::UTC));
    QCOMPARE(eventHandler.getTime(true, QDateTime::fromMSecsSinceEpoch(1234, Qt::UTC)), QDateTime::fromMSecsSinceEpoch(1234, Qt::UTC));
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

void EventHandlerTest::highlighted()
{
    auto event = room->messageEvents().at(2).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.isHighlighted(), true);

    event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.isHighlighted(), false);
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

void EventHandlerTest::body()
{
    auto event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getRichBody(), QStringLiteral("<b>This is an example<br>text message</b>"));
    QCOMPARE(eventHandler.getRichBody(true), QStringLiteral("<b>This is an example text message</b>"));
    QCOMPARE(eventHandler.getPlainBody(), QStringLiteral("This is an example\ntext message"));
    QCOMPARE(eventHandler.getPlainBody(true), QStringLiteral("This is an example text message"));
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

void EventHandlerTest::linkPreviewer()
{
    auto event = room->messageEvents().at(2).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getLinkPreviewer()->url(), QUrl("https://kde.org"_ls));

    event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getLinkPreviewer(), nullptr);
}

void EventHandlerTest::reactions()
{
    auto event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getReactions()->rowCount(), 1);
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

void EventHandlerTest::replyId()
{
    auto event = room->messageEvents().at(5).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getReplyId(), QStringLiteral("$153456789:example.org"));

    event = room->messageEvents().at(0).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getReplyId(), QStringLiteral(""));
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

void EventHandlerTest::replyBody()
{
    auto event = room->messageEvents().at(5).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getReplyRichBody(), QStringLiteral("<b>This is an example<br>text message</b>"));
    QCOMPARE(eventHandler.getReplyRichBody(true), QStringLiteral("<b>This is an example text message</b>"));
    QCOMPARE(eventHandler.getReplyPlainBody(), QStringLiteral("This is an example\ntext message"));
    QCOMPARE(eventHandler.getReplyPlainBody(true), QStringLiteral("This is an example text message"));
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

void EventHandlerTest::location()
{
    auto event = room->messageEvents().at(7).get();
    eventHandler.setEvent(event);

    QCOMPARE(eventHandler.getLatitude(), QStringLiteral("51.7035").toFloat());
    QCOMPARE(eventHandler.getLongitude(), QStringLiteral("-1.14394").toFloat());
    QCOMPARE(eventHandler.getLocationAssetType(), QStringLiteral("m.pin"));
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

QTEST_MAIN(EventHandlerTest)
#include "eventhandlertest.moc"
