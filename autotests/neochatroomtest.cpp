// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include "neochatconnection.h"

#include "testutils.h"

using namespace Quotient;

class NeoChatRoomTest : public QObject {
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestUtils::TestRoom *room = nullptr;

private Q_SLOTS:
    void initTestCase();
    void eventTest();
};

void NeoChatRoomTest::initTestCase()
{
    auto connection = new NeoChatConnection;
    Connection::makeMockConnection(connection, QStringLiteral("@bob:kde.org"));
    room = new TestUtils::TestRoom(connection, QStringLiteral("#myroom:kde.org"), "test-min-sync.json"_ls);
}

void NeoChatRoomTest::eventTest()
{
    QCOMPARE(room->timelineSize(), 1);
}

QTEST_GUILESS_MAIN(NeoChatRoomTest)
#include "neochatroomtest.moc"
