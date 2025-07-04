// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <Quotient/connection.h>
#include <Quotient/quotient_common.h>
#include <Quotient/syncdata.h>

#include "enums/delegatetype.h"
#include "models/timelinemessagemodel.h"
#include "neochatroom.h"

#include "testutils.h"

using namespace Quotient;

class TimelineMessageModelTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TimelineMessageModel *model = nullptr;

private Q_SLOTS:
    void initTestCase();
    void init();

    void switchEmptyRoom();
    void switchSyncedRoom();
    void simpleTimeline();
    void syncNewEvents();
    void pendingEvent();
    void disconnect();
    void idToRow();

    void cleanup();
};

void TimelineMessageModelTest::initTestCase()
{
    connection = Connection::makeMockConnection(u"@bob:kde.org"_s);
}

void TimelineMessageModelTest::init()
{
    QCOMPARE(model, nullptr);
    model = new TimelineMessageModel;
}

// Make sure that basic empty rooms can be switched without crashing.
void TimelineMessageModelTest::switchEmptyRoom()
{
    auto firstRoom = new TestUtils::TestRoom(connection, u"#firstRoom:kde.org"_s);
    auto secondRoom = new TestUtils::TestRoom(connection, u"#secondRoom:kde.org"_s);

    QSignalSpy spy(model, SIGNAL(roomChanged(NeoChatRoom *, NeoChatRoom *)));

    QCOMPARE(model->room(), nullptr);
    model->setRoom(firstRoom);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(model->room()->id(), u"#firstRoom:kde.org"_s);
    model->setRoom(secondRoom);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(model->room()->id(), u"#secondRoom:kde.org"_s);
    model->setRoom(nullptr);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(model->room(), nullptr);
}

// Make sure that rooms with some events can be switched without crashing
void TimelineMessageModelTest::switchSyncedRoom()
{
    auto firstRoom = new TestUtils::TestRoom(connection, u"#firstRoom:kde.org"_s, u"test-messageventmodel-sync.json"_s);
    auto secondRoom = new TestUtils::TestRoom(connection, u"#secondRoom:kde.org"_s, u"test-messageventmodel-sync.json"_s);

    QSignalSpy spy(model, SIGNAL(roomChanged(NeoChatRoom *, NeoChatRoom *)));

    QCOMPARE(model->room(), nullptr);
    model->setRoom(firstRoom);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(model->room()->id(), u"#firstRoom:kde.org"_s);
    model->setRoom(secondRoom);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(model->room()->id(), u"#secondRoom:kde.org"_s);
    model->setRoom(nullptr);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(model->room(), nullptr);
}

void TimelineMessageModelTest::simpleTimeline()
{
    auto room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s, u"test-messageventmodel-sync.json"_s);

    model->setRoom(room);
    QCOMPARE(model->rowCount(), 2);

    QCOMPARE(model->data(model->index(0), TimelineMessageModel::DelegateTypeRole), DelegateType::State);
    QCOMPARE(model->data(model->index(0)), u"changed their display name to Example Changed"_s);

    QCOMPARE(model->data(model->index(1)), u"<b>This is an example<br>text message</b>"_s);
    QCOMPARE(model->data(model->index(1), TimelineMessageModel::DelegateTypeRole), DelegateType::Message);
    QCOMPARE(model->data(model->index(1), TimelineMessageModel::EventIdRole), u"$153456789:example.org"_s);

    QTest::ignoreMessage(QtWarningMsg, "Index QModelIndex(-1,-1,0x0,QObject(0x0)) is not valid (expected valid)");
    QCOMPARE(model->data(model->index(-1)), QVariant());
    QTest::ignoreMessage(QtWarningMsg, "Index QModelIndex(-1,-1,0x0,QObject(0x0)) is not valid (expected valid)");
    QCOMPARE(model->data(model->index(model->rowCount())), QVariant());
}

// Sync some events into the TimelineMessageModel's current room and don't crash.
void TimelineMessageModelTest::syncNewEvents()
{
    auto room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s);
    QSignalSpy spy(room, SIGNAL(aboutToAddNewMessages(Quotient::RoomEventsRange)));

    model->setRoom(room);
    QCOMPARE(model->rowCount(), 0);

    room->syncNewEvents(u"test-messageventmodel-sync.json"_s);

    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(spy.count(), 1);
}

// Check the adding of pending events to the room doesn't cause any issues in the model.
void TimelineMessageModelTest::pendingEvent()
{
    QSignalSpy spyInsert(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
    QSignalSpy spyRemove(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
    QSignalSpy spyChanged(model, SIGNAL(dataChanged(const QModelIndex, const QModelIndex, const QList<int> &)));

    auto room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s);
    model->setRoom(room);
    QCOMPARE(model->rowCount(), 0);

#if Quotient_VERSION_MINOR > 9
    auto txnId = room->postText("New plain message"_L1);
#else
    auto txnId = room->postPlainText("New plain message"_L1);
#endif
    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(spyInsert.count(), 1);

    room->discardMessage(txnId);
    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(spyRemove.count(), 1);

#if Quotient_VERSION_MINOR > 9
    txnId = room->postText("New plain message"_L1);
#else
    txnId = room->postPlainText("New plain message"_L1);
#endif
    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(spyInsert.count(), 2);

    // We need to manually set the transaction ID of the new message as it will be
    // different every time.
    QFile testSyncFile;
    testSyncFile.setFileName(QStringLiteral(DATA_DIR) + u'/' + u"test-pending-sync.json"_s);
    testSyncFile.open(QIODevice::ReadOnly);
    auto testSyncJson = QJsonDocument::fromJson(testSyncFile.readAll());
    auto root = testSyncJson.object();
    auto timeline = root["timeline"_L1].toObject();
    auto events = timeline["events"_L1].toArray();
    auto firstEvent = events[0].toObject();
    firstEvent.insert("unsigned"_L1, QJsonObject{{"transaction_id"_L1, txnId}});
    events[0] = firstEvent;
    timeline.insert("events"_L1, events);
    root.insert("timeline"_L1, timeline);
    testSyncJson.setObject(root);
    SyncRoomData roomData(u"@bob:kde.org"_s, JoinState::Join, testSyncJson.object());
    room->update(std::move(roomData));

    QCOMPARE(model->rowCount(), 1);
    // The model will throw multiple data changed signals we need the one that refreshes
    // the IsPendingRole.
    QCOMPARE(spyChanged.count() > 0, true);
    auto isPendingChanged = false;
    for (auto signal : spyChanged) {
        auto roles = signal.at(2).toList();
        if (roles.contains(TimelineMessageModel::IsPendingRole)) {
            isPendingChanged = true;
        }
    }
    QCOMPARE(isPendingChanged, true);
}

// Make sure that the signals are disconnecting correctly when a room is switched.
void TimelineMessageModelTest::disconnect()
{
    auto room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s);
    model->setRoom(room);

    QSignalSpy spy(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));

    model->setRoom(nullptr);
    room->syncNewEvents(u"test-messageventmodel-sync.json"_s);

    QCOMPARE(spy.count(), 0);
}

void TimelineMessageModelTest::idToRow()
{
    auto room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s, u"test-min-sync.json"_s);
    model->setRoom(room);

    QCOMPARE(model->indexforEventId(u"$153456789:example.org"_s).row(), 0);
}

void TimelineMessageModelTest::cleanup()
{
    delete model;
    model = nullptr;
    QCOMPARE(model, nullptr);
}

QTEST_MAIN(TimelineMessageModelTest)
#include "timelinemessagemodeltest.moc"
