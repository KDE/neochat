// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <Quotient/connection.h>
#include <Quotient/quotient_common.h>
#include <Quotient/syncdata.h>

#include "enums/delegatetype.h"
#include "models/messageeventmodel.h"
#include "neochatroom.h"

#include "testutils.h"

using namespace Quotient;

class MessageEventModelTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    MessageEventModel *model = nullptr;

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

void MessageEventModelTest::initTestCase()
{
    connection = Connection::makeMockConnection(QStringLiteral("@bob:kde.org"));
}

void MessageEventModelTest::init()
{
    QCOMPARE(model, nullptr);
    model = new MessageEventModel;
}

// Make sure that basic empty rooms can be switched without crashing.
void MessageEventModelTest::switchEmptyRoom()
{
    auto firstRoom = new TestUtils::TestRoom(connection, QStringLiteral("#firstRoom:kde.org"));
    auto secondRoom = new TestUtils::TestRoom(connection, QStringLiteral("#secondRoom:kde.org"));

    QSignalSpy spy(model, SIGNAL(roomChanged()));

    QCOMPARE(model->room(), nullptr);
    model->setRoom(firstRoom);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(model->room()->id(), QStringLiteral("#firstRoom:kde.org"));
    model->setRoom(secondRoom);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(model->room()->id(), QStringLiteral("#secondRoom:kde.org"));
    model->setRoom(nullptr);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(model->room(), nullptr);
}

// Make sure that rooms with some events can be switched without crashing
void MessageEventModelTest::switchSyncedRoom()
{
    auto firstRoom = new TestUtils::TestRoom(connection, QStringLiteral("#firstRoom:kde.org"), QLatin1String("test-messageventmodel-sync.json"));
    auto secondRoom = new TestUtils::TestRoom(connection, QStringLiteral("#secondRoom:kde.org"), QLatin1String("test-messageventmodel-sync.json"));

    QSignalSpy spy(model, SIGNAL(roomChanged()));

    QCOMPARE(model->room(), nullptr);
    model->setRoom(firstRoom);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(model->room()->id(), QStringLiteral("#firstRoom:kde.org"));
    model->setRoom(secondRoom);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(model->room()->id(), QStringLiteral("#secondRoom:kde.org"));
    model->setRoom(nullptr);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(model->room(), nullptr);
}

void MessageEventModelTest::simpleTimeline()
{
    auto room = new TestUtils::TestRoom(connection, QStringLiteral("#myroom:kde.org"), QLatin1String("test-messageventmodel-sync.json"));

    model->setRoom(room);
    QCOMPARE(model->rowCount(), 2);

    QCOMPARE(model->data(model->index(0), MessageEventModel::DelegateTypeRole), DelegateType::State);
    QCOMPARE(model->data(model->index(0)), QStringLiteral("changed their display name to Example Changed"));

    QCOMPARE(model->data(model->index(1)), QStringLiteral("<b>This is an example<br>text message</b>"));
    QCOMPARE(model->data(model->index(1), MessageEventModel::DelegateTypeRole), DelegateType::Message);
    QCOMPARE(model->data(model->index(1), MessageEventModel::EventIdRole), QStringLiteral("$153456789:example.org"));

    QTest::ignoreMessage(QtWarningMsg, "Index QModelIndex(-1,-1,0x0,QObject(0x0)) is not valid (expected valid)");
    QCOMPARE(model->data(model->index(-1)), QVariant());
    QTest::ignoreMessage(QtWarningMsg, "Index QModelIndex(-1,-1,0x0,QObject(0x0)) is not valid (expected valid)");
    QCOMPARE(model->data(model->index(model->rowCount())), QVariant());
}

// Sync some events into the MessageEventModel's current room and don't crash.
void MessageEventModelTest::syncNewEvents()
{
    auto room = new TestUtils::TestRoom(connection, QStringLiteral("#myroom:kde.org"));
    QSignalSpy spy(room, SIGNAL(aboutToAddNewMessages(Quotient::RoomEventsRange)));

    model->setRoom(room);
    QCOMPARE(model->rowCount(), 0);

    room->syncNewEvents(QLatin1String("test-messageventmodel-sync.json"));

    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(spy.count(), 1);
}

// Check the adding of pending events to the room doesn't cause any issues in the model.
void MessageEventModelTest::pendingEvent()
{
    QSignalSpy spyInsert(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
    QSignalSpy spyRemove(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
    QSignalSpy spyChanged(model, SIGNAL(dataChanged(const QModelIndex, const QModelIndex, const QList<int> &)));

    auto room = new TestUtils::TestRoom(connection, QStringLiteral("#myroom:kde.org"));
    model->setRoom(room);
    QCOMPARE(model->rowCount(), 0);

    auto txnId = room->postPlainText("New plain message"_ls);
    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(spyInsert.count(), 1);

    room->discardMessage(txnId);
    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(spyRemove.count(), 1);

    txnId = room->postPlainText("New plain message"_ls);
    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(spyInsert.count(), 2);

    // We need to manually set the transaction ID of the new message as it will be
    // different every time.
    QFile testSyncFile;
    testSyncFile.setFileName(QLatin1String(DATA_DIR) + u'/' + QLatin1String("test-pending-sync.json"));
    testSyncFile.open(QIODevice::ReadOnly);
    auto testSyncJson = QJsonDocument::fromJson(testSyncFile.readAll());
    auto root = testSyncJson.object();
    auto timeline = root["timeline"_ls].toObject();
    auto events = timeline["events"_ls].toArray();
    auto firstEvent = events[0].toObject();
    firstEvent.insert(QLatin1String("unsigned"), QJsonObject{{QLatin1String("transaction_id"), txnId}});
    events[0] = firstEvent;
    timeline.insert("events"_ls, events);
    root.insert("timeline"_ls, timeline);
    testSyncJson.setObject(root);
    SyncRoomData roomData(QStringLiteral("@bob:kde.org"), JoinState::Join, testSyncJson.object());
    room->update(std::move(roomData));

    QCOMPARE(model->rowCount(), 1);
    // The model will throw multiple data changed signals we need the one that refreshes
    // the IsPendingRole.
    QCOMPARE(spyChanged.count() > 0, true);
    auto isPendingChanged = false;
    for (auto signal : spyChanged) {
        auto roles = signal.at(2).toList();
        if (roles.contains(MessageEventModel::IsPendingRole)) {
            isPendingChanged = true;
        }
    }
    QCOMPARE(isPendingChanged, true);
}

// Make sure that the signals are disconnecting correctly when a room is switched.
void MessageEventModelTest::disconnect()
{
    auto room = new TestUtils::TestRoom(connection, QStringLiteral("#myroom:kde.org"));
    model->setRoom(room);

    QSignalSpy spy(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));

    model->setRoom(nullptr);
    room->syncNewEvents(QLatin1String("test-messageventmodel-sync.json"));

    QCOMPARE(spy.count(), 0);
}

void MessageEventModelTest::idToRow()
{
    auto room = new TestUtils::TestRoom(connection, QStringLiteral("#myroom:kde.org"), QLatin1String("test-min-sync.json"));
    model->setRoom(room);

    QCOMPARE(model->eventIdToRow(QStringLiteral("$153456789:example.org")), 0);
}

void MessageEventModelTest::cleanup()
{
    delete model;
    model = nullptr;
    QCOMPARE(model, nullptr);
}

QTEST_MAIN(MessageEventModelTest)
#include "messageeventmodeltest.moc"
