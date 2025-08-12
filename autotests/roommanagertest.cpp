// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QSignalSpy>
#include <QTest>
#include <QVariantList>

#include "accountmanager.h"
#include "models/actionsmodel.h"
#include "roommanager.h"

#include "server.h"
#include "testutils.h"

using namespace Quotient;

class RoomManagerTest : public QObject
{
    Q_OBJECT

private:
    NeoChatConnection *connection = nullptr;
    NeoChatRoom *room = nullptr;

    Server server;

private Q_SLOTS:
    void initTestCase();
    void testMaximizeMedia();
};

void RoomManagerTest::initTestCase()
{
    Connection::setRoomType<NeoChatRoom>();
    server.start();
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("neochat"));
    auto accountManager = new AccountManager(true);
    QSignalSpy spy(accountManager, &AccountManager::connectionAdded);
    connection = dynamic_cast<NeoChatConnection *>(accountManager->accounts()->front());
    QVERIFY(connection);
    auto roomId = server.createRoom(u"@user:localhost:1234"_s);

    QSignalSpy syncSpy(connection, &Connection::syncDone);
    // We need to wait for two syncs, as the next one won't have the changes yet
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
    room = dynamic_cast<NeoChatRoom *>(connection->room(roomId));
    QVERIFY(room);
    RoomManager::instance().setConnection(connection);
    QSignalSpy roomSpy(&RoomManager::instance(), &RoomManager::currentRoomChanged);
    RoomManager::instance().resolveResource(room->id());
    QVERIFY(roomSpy.size() > 0);
}

void RoomManagerTest::testMaximizeMedia()
{
    QSignalSpy spy(&RoomManager::instance(), &RoomManager::showMaximizedMedia);
    QSignalSpy syncSpy(connection, &Connection::syncDone);

    QTest::ignoreMessage(QtMsgType::QtWarningMsg, "Tried to open media for empty event id");
    RoomManager::instance().maximizeMedia(QString());
    QVERIFY(!spy.wait(10));

    QTest::ignoreMessage(QtMsgType::QtWarningMsg, "Tried to open media for unknown event id \"Doesn't exist\"");
    RoomManager::instance().maximizeMedia(u"Doesn't exist"_s);
    QVERIFY(!spy.wait(10));

    const auto eventWithoutMedia = server.sendEvent(room->id(),
                                                    u"m.room.message"_s,
                                                    QJsonObject({
                                                        {u"body"_s, u"Foo"_s},
                                                        {u"format"_s, u"org.matrix.custom.html"_s},
                                                        {u"formatted_body"_s, u"Foo"_s},
                                                        {u"msgtype"_s, u"m.text"_s},
                                                    }));
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
    QTest::ignoreMessage(QtMsgType::QtWarningMsg, u"Tried to open media for unknown event id \"%1\""_s.arg(eventWithoutMedia).toLatin1().data());
    RoomManager::instance().maximizeMedia(eventWithoutMedia);
    QVERIFY(!spy.wait(10));

    // NOTE: This is supposed to test that maximizing pending media works correctly. This probably doesn't work in the UI yet, but at least the backend supports
    // it. If the server ever learns how to process events, this becomes pointless and we need to find a way of preventing *these* events from arriving
    auto pendingEventWithoutMedia = room->postText(u"Hello"_s);
    QTest::ignoreMessage(QtMsgType::QtWarningMsg, u"Tried to open media for unknown event id \"%1\""_s.arg(pendingEventWithoutMedia).toLatin1().data());
    RoomManager::instance().maximizeMedia(pendingEventWithoutMedia);
    QVERIFY(!spy.wait(10));

    const auto eventWithMedia = server.sendEvent(room->id(),
                                                 u"m.room.message"_s,
                                                 QJsonObject({
                                                     {u"body"_s, u"Foo"_s},
                                                     {u"filename"_s, u"foo.jpg"_s},
                                                     {u"info"_s,
                                                      QJsonObject{
                                                          {u"h"_s, 1000},
                                                          {u"w"_s, 2000},
                                                          {u"size"_s, 10000},
                                                          {u"mimetype"_s, u"image/png"_s},
                                                      }},
                                                     {u"msgtype"_s, u"m.image"_s},
                                                     {u"url"_s, u"mxc://foo.bar/asdf"_s},
                                                 }));
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
    RoomManager::instance().maximizeMedia(eventWithMedia);
    QVERIFY(spy.size() == 1);
    QVERIFY(spy[0][0] == 0);

    auto pendingEventWithMedia = room->postJson(u"m.room.message"_s,
                                                QJsonObject({
                                                    {u"body"_s, u"Foo"_s},
                                                    {u"filename"_s, u"foo.jpg"_s},
                                                    {u"info"_s,
                                                     QJsonObject{
                                                         {u"h"_s, 1000},
                                                         {u"w"_s, 2000},
                                                         {u"size"_s, 10000},
                                                         {u"mimetype"_s, u"image/png"_s},
                                                     }},
                                                    {u"msgtype"_s, u"m.image"_s},
                                                    {u"url"_s, u"mxc://foo.bar/asdf"_s},
                                                }));
    RoomManager::instance().maximizeMedia(pendingEventWithMedia);
    QVERIFY(spy.size() == 2);
    QVERIFY(spy[1][0] == 0);
}

QTEST_MAIN(RoomManagerTest)
#include "roommanagertest.moc"
