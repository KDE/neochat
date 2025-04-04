// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include "models/reactionmodel.h"

#include <Quotient/events/roommessageevent.h>

#include "models/messagecontentmodel.h"
#include "testutils.h"

using namespace Quotient;

class ReactionModelTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestUtils::TestRoom *room = nullptr;
    MessageContentModel *parentModel;

private Q_SLOTS:
    void initTestCase();

    void basicReaction();
    void newReaction();
};

void ReactionModelTest::initTestCase()
{
    connection = Connection::makeMockConnection(u"@bob:kde.org"_s);
    room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s, u"test-reactionmodel-sync.json"_s);
    parentModel = new MessageContentModel(room, "123456"_L1);
}

void ReactionModelTest::basicReaction()
{
    auto event = eventCast<const RoomMessageEvent>(room->messageEvents().at(0).get());
    auto model = ReactionModel(parentModel, event->id(), room);

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0), ReactionModel::TextContentRole), u"<span style=\"font-family: 'emoji';\">👍</span>"_s);
    QCOMPARE(model.data(model.index(0), ReactionModel::ReactionRole), u"👍"_s);
    QCOMPARE(model.data(model.index(0), ReactionModel::ToolTipRole), u"Alice Margatroid reacted with <span style=\"font-family: 'emoji';\">👍</span>"_s);
    QCOMPARE(model.data(model.index(0), ReactionModel::HasLocalMember), false);
}

void ReactionModelTest::newReaction()
{
    auto event = eventCast<const RoomMessageEvent>(room->messageEvents().at(0).get());
    auto model = new ReactionModel(parentModel, event->id(), room);

    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(model->data(model->index(0), ReactionModel::ToolTipRole), u"Alice Margatroid reacted with <span style=\"font-family: 'emoji';\">👍</span>"_s);

    QSignalSpy spy(model, SIGNAL(modelReset()));

    room->syncNewEvents(u"test-reactionmodel-extra-sync.json"_s);
    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(spy.count(), 2); // Once for each of the 2 new reactions.
    QCOMPARE(model->data(model->index(1), ReactionModel::ReactionRole), u"😆"_s);
    QCOMPARE(model->data(model->index(0), ReactionModel::ToolTipRole),
             u"Alice Margatroid and Bob reacted with <span style=\"font-family: 'emoji';\">👍</span>"_s);

    delete model;
}

QTEST_MAIN(ReactionModelTest)
#include "reactionmodeltest.moc"
