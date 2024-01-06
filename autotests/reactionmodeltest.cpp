// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include "models/reactionmodel.h"

#include <Quotient/events/roommessageevent.h>

#include "testutils.h"

using namespace Quotient;

class ReactionModelTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestUtils::TestRoom *room = nullptr;

private Q_SLOTS:
    void initTestCase();

    void nullModel();
    void basicReaction();
    void newReaction();
};

void ReactionModelTest::initTestCase()
{
    connection = Connection::makeMockConnection(QStringLiteral("@bob:kde.org"));
    room = new TestUtils::TestRoom(connection, QStringLiteral("#myroom:kde.org"), QLatin1String("test-reactionmodel-sync.json"));
}

void ReactionModelTest::nullModel()
{
    auto model = ReactionModel(nullptr, nullptr);

    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.data(model.index(0), ReactionModel::TextContentRole), QVariant());
}

void ReactionModelTest::basicReaction()
{
    auto event = eventCast<const RoomMessageEvent>(room->messageEvents().at(0).get());
    auto model = ReactionModel(event, room);

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0), ReactionModel::TextContentRole), QStringLiteral("<span style=\"font-family: 'emoji';\">👍</span>"));
    QCOMPARE(model.data(model.index(0), ReactionModel::ReactionRole), QStringLiteral("👍"));
    QCOMPARE(model.data(model.index(0), ReactionModel::ToolTipRole),
             QStringLiteral("@alice:matrix.org reacted with <span style=\"font-family: 'emoji';\">👍</span>"));
    auto authorList = QVariantList{room->getUser(room->user(QStringLiteral("@alice:matrix.org")))};
    QCOMPARE(model.data(model.index(0), ReactionModel::AuthorsRole), authorList);
    QCOMPARE(model.data(model.index(0), ReactionModel::HasLocalUser), false);
}

void ReactionModelTest::newReaction()
{
    auto event = eventCast<const RoomMessageEvent>(room->messageEvents().at(0).get());
    auto model = new ReactionModel(event, room);

    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(model->data(model->index(0), ReactionModel::ToolTipRole),
             QStringLiteral("@alice:matrix.org reacted with <span style=\"font-family: 'emoji';\">👍</span>"));

    QSignalSpy spy(model, SIGNAL(modelReset()));

    room->syncNewEvents(QLatin1String("test-reactionmodel-extra-sync.json"));
    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(spy.count(), 2); // Once for each of the 2 new reactions.
    QCOMPARE(model->data(model->index(1), ReactionModel::ReactionRole), QStringLiteral("😆"));
    QCOMPARE(model->data(model->index(0), ReactionModel::ToolTipRole),
             QStringLiteral("@alice:matrix.org and @bob:example.org reacted with <span style=\"font-family: 'emoji';\">👍</span>"));

    delete model;
}

QTEST_MAIN(ReactionModelTest)
#include "reactionmodeltest.moc"
