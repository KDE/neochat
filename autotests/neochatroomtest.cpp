// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include "neochatroom.h"

#include <Quotient/connection.h>
#include <Quotient/quotient_common.h>
#include <Quotient/syncdata.h>

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

    QFile testMinSyncFile;
    testMinSyncFile.setFileName(QLatin1String(DATA_DIR) + u'/' + QLatin1String("test-min-sync.json"));
    testMinSyncFile.open(QIODevice::ReadOnly);
    const auto testMinSyncJson = QJsonDocument::fromJson(testMinSyncFile.readAll());
    SyncRoomData roomData(QStringLiteral("@bob:kde.org"), JoinState::Join, testMinSyncJson.object());
    room->update(std::move(roomData));
}

void NeoChatRoomTest::subtitleTextTest()
{
    QCOMPARE(room->timelineSize(), 1);
    QCOMPARE(room->lastEventToString(), QStringLiteral("@example:example.org: This is an example\ntext message"));
}

void NeoChatRoomTest::eventTest()
{
    QCOMPARE(room->timelineSize(), 1);
}

QTEST_GUILESS_MAIN(NeoChatRoomTest)
#include "neochatroomtest.moc"
