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

class MessageEventModelTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    MessageEventModel *model = nullptr;

    TestRoom *setupTestRoom(const QString &syncFileName);

private Q_SLOTS:
    void initTestCase();
    void init();

    void switchRoom();
    void simpleTimeline();
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

// Make sure that basic rooms can be switched without crashing.
void MessageEventModelTest::switchRoom()
{
    TestRoom *firstRoom = new TestRoom(connection, QStringLiteral("#firstRoom:kde.org"), JoinState::Join);
    TestRoom *secondRoom = new TestRoom(connection, QStringLiteral("#secondRoom:kde.org"), JoinState::Join);

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
    auto room = setupTestRoom(QLatin1String("test-min-sync.json"));

    model->setRoom(room);
    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(model->data(model->index(0)), QStringLiteral("<b>This is an example<br>text message</b>"));
    QCOMPARE(model->data(model->index(0), MessageEventModel::DelegateTypeRole), DelegateType::Message);
    QCOMPARE(model->data(model->index(0), MessageEventModel::PlainText), QStringLiteral("This is an example\ntext message"));
    QCOMPARE(model->data(model->index(0), MessageEventModel::EventIdRole), QStringLiteral("$153456789:example.org"));

    QTest::ignoreMessage(QtWarningMsg, "Index QModelIndex(-1,-1,0x0,QObject(0x0)) is not valid (expected valid)");
    QCOMPARE(model->data(model->index(-1)), QVariant());
    QTest::ignoreMessage(QtWarningMsg, "Index QModelIndex(-1,-1,0x0,QObject(0x0)) is not valid (expected valid)");
    QCOMPARE(model->data(model->index(1)), QVariant());
}

void MessageEventModelTest::idToRow()
{
    auto room = setupTestRoom(QLatin1String("test-min-sync.json"));
    model->setRoom(room);

    QCOMPARE(model->eventIdToRow(QStringLiteral("$153456789:example.org")), 0);
}

void MessageEventModelTest::cleanup()
{
    delete model;
    model = nullptr;
    QCOMPARE(model, nullptr);
}

TestRoom *MessageEventModelTest::setupTestRoom(const QString &syncFileName)
{
    auto room = new TestRoom(connection, QStringLiteral("#myroom:kde.org"), JoinState::Join);

    if (!syncFileName.isEmpty()) {
        QFile testSyncFile;
        testSyncFile.setFileName(QLatin1String(DATA_DIR) + u'/' + syncFileName);
        testSyncFile.open(QIODevice::ReadOnly);
        const auto testSyncJson = QJsonDocument::fromJson(testSyncFile.readAll());
        SyncRoomData roomData(QStringLiteral("@bob:kde.org"), JoinState::Join, testSyncJson.object());
        room->update(std::move(roomData));
    }

    return room;
}

QTEST_GUILESS_MAIN(MessageEventModelTest)
#include "messageeventmodeltest.moc"
