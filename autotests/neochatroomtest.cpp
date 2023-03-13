// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include "neochatroom.h"

#include <connection.h>
#include <quotient_common.h>
#include <syncdata.h>

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

class NeoChatRoomTest : public QObject {
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestRoom *room = nullptr;

private Q_SLOTS:
    void initTestCase();
    void subtitleTextTest();
    void eventTest();
};

void NeoChatRoomTest::initTestCase()
{
    connection = Connection::makeMockConnection(QStringLiteral("@bob:kde.org"));
    room = new TestRoom(connection, QStringLiteral("#myroom:kde.org"), JoinState::Join);

    auto json = QJsonDocument::fromJson(R"EVENT({
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
          "body": "This is an **example** text message",
          "format": "org.matrix.custom.html",
          "formatted_body": "<b>This is an example text message</b>",
          "msgtype": "m.text"
        },
        "event_id": "$143273582443PhrSn:example.org",
        "origin_server_ts": 1432735824654,
        "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
        "sender": "@example:example.org",
        "type": "m.room.message",
        "unsigned": {
          "age": 1235
        }
      }
    ],
    "limited": true,
    "prev_batch": "t34-23535_0_0"
  }
})EVENT");
    SyncRoomData roomData(QStringLiteral("@bob:kde.org"), JoinState::Join, json.object());
    room->update(std::move(roomData));
}

void NeoChatRoomTest::subtitleTextTest()
{
    QCOMPARE(room->timelineSize(), 1);
    QCOMPARE(room->lastEventToString(), QStringLiteral("@example:example.org: This is an example text message"));
}

void NeoChatRoomTest::eventTest()
{
    QCOMPARE(room->timelineSize(), 1);
}

QTEST_MAIN(NeoChatRoomTest)
#include "neochatroomtest.moc"
