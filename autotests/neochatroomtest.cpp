// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <Quotient/connection.h>
#include <Quotient/quotient_common.h>
#include <Quotient/syncdata.h>

#include <KLocalizedString>

#include "accountmanager.h"
#include "server.h"
#include "testutils.h"

using namespace Quotient;

class NeoChatRoomTest : public QObject {
    Q_OBJECT

private:
    Connection *connection = nullptr;
    NeoChatRoom *room = nullptr;
    Server server;

private Q_SLOTS:
    void initTestCase();
    void eventTest();
};

void NeoChatRoomTest::initTestCase()
{
    Connection::setRoomType<NeoChatRoom>();
    server.start();
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("neochat"));
    auto accountManager = new AccountManager(true, this);
    QSignalSpy spy(accountManager, &AccountManager::connectionAdded);
    connection = dynamic_cast<NeoChatConnection *>(accountManager->accounts()->front());

    const auto roomId = server.createRoom(u"@user:localhost:1234"_s);
    server.sendEvent(roomId,
                     u"m.room.message"_s,
                     QJsonObject{
                         {u"body"_s, u"foo"_s},
                         {u"msgtype"_s, u"m.text"_s},
                     });

    QSignalSpy syncSpy(connection, &Connection::syncDone);
    // We need to wait for two syncs, as the next one won't have the changes yet
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
    room = dynamic_cast<NeoChatRoom *>(connection->room(roomId));
    QVERIFY(room);
}

void NeoChatRoomTest::eventTest()
{
    QCOMPARE(room->timelineSize(), 1);
}

QTEST_GUILESS_MAIN(NeoChatRoomTest)
#include "neochatroomtest.moc"
