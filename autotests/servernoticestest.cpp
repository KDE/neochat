// SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <KLocalizedString>

#include <Quotient/connection.h>
#include <Quotient/eventstats.h>
#include <Quotient/quotient_common.h>
#include <Quotient/syncdata.h>

#include "accountmanager.h"
#include "neochatroom.h"
#include "roommanager.h"
#include "server.h"

#include "testutils.h"

using namespace Quotient;

class ServerNoticesTest : public QObject
{
    Q_OBJECT

private:
    NeoChatConnection *connection = nullptr;
    Server server;

private Q_SLOTS:
    void initTestCase();
    void test();
};

void ServerNoticesTest::initTestCase()
{
    Connection::setRoomType<NeoChatRoom>();
    server.start();
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("neochat"));
    auto accountManager = new AccountManager(true);
    QSignalSpy spy(accountManager, &AccountManager::connectionAdded);
    connection = dynamic_cast<NeoChatConnection *>(accountManager->accounts()->front());
    QVERIFY(connection);
    auto roomId = server.createRoom(u"@user:localhost:1234"_s);
    RoomManager::instance().setConnection(connection);

    QSignalSpy syncSpy(connection, &Connection::syncDone);
    // We need to wait for two syncs, as the next one won't have the changes yet
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
    auto room = dynamic_cast<NeoChatRoom *>(connection->room(roomId));
    QVERIFY(room);
}

void ServerNoticesTest::test()
{
    auto roomTreeModel = RoomManager::instance().roomTreeModel();
    QCOMPARE(roomTreeModel->rowCount(roomTreeModel->index(NeoChatRoomType::ServerNotice, 0)), 0);
    auto sortFilterRoomTreeModel = RoomManager::instance().sortFilterRoomTreeModel();
    const auto roomId = server.createServerNoticesRoom(u"@user:localhost:1234"_s);
    QSignalSpy syncSpy(connection, &Connection::syncDone);
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
    const auto room = dynamic_cast<NeoChatRoom *>(connection->room(roomId));
    QVERIFY(connection->room(roomId)->isServerNoticeRoom());
    QCOMPARE(roomTreeModel->rowCount(roomTreeModel->index(NeoChatRoomType::ServerNotice, 0)), 1);
    QCOMPARE(sortFilterRoomTreeModel->mapFromSource(roomTreeModel->indexForRoom(room)).parent().row(), 1 /* Below the normal room */);
    server.sendEvent(roomId,
                     u"m.room.message"_s,
                     QJsonObject{
                         {u"body"_s, u"Foo"_s},
                         {u"format"_s, u"org.matrix.custom.html"_s},
                         {u"formatted_body"_s, u"Foo"_s},
                         {u"msgtype"_s, u"m.text"_s},
                     });
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
    sortFilterRoomTreeModel->invalidate();
    QCOMPARE(sortFilterRoomTreeModel->mapFromSource(roomTreeModel->indexForRoom(room)).parent().row(), 0);
    room->markAllMessagesAsRead();
    QCOMPARE(sortFilterRoomTreeModel->mapFromSource(roomTreeModel->indexForRoom(room)).parent().row(), 1 /* Below the normal room */);
}

QTEST_GUILESS_MAIN(ServerNoticesTest)
#include "servernoticestest.moc"
