// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <Quotient/connection.h>
#include <Quotient/quotient_common.h>
#include <Quotient/syncdata.h>

#include "events/pollevent.h"
#include "pollblock.h"
#include "testutils.h"

using namespace Quotient;
using namespace Blocks;

class PollBlockTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestUtils::TestRoom *room = nullptr;

private Q_SLOTS:
    void initTestCase();
    void nullObject();
    void poll();
};

void PollBlockTest::initTestCase()
{
    connection = Connection::makeMockConnection(u"@bob:kde.org"_s);
    room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s, u"test-pollhandlerstart-sync.json"_s);
}

// Basically don't crash.
void PollBlockTest::nullObject()
{
    auto pollBlock = PollBlock(Poll, {}, nullptr, this);

    QCOMPARE(pollBlock.hasEnded(), false);
    QCOMPARE(pollBlock.numAnswers(), 0);
    QCOMPARE(pollBlock.question(), QString());
    QCOMPARE(pollBlock.kind(), PollKind::Disclosed);
}

void PollBlockTest::poll()
{
    auto startEvent = eventCast<const PollStartEvent>(room->messageEvents().at(0).get());
    auto pollBlock = PollBlock(Poll, startEvent->id(), room, this);

    QList<Quotient::EventContent::Answer> options = {EventContent::Answer{"option1"_L1, "option1"_L1}, EventContent::Answer{"option2"_L1, "option2"_L1}};

    const auto answer0 = pollBlock.answerAtRow(0);
    const auto answer1 = pollBlock.answerAtRow(1);
    QCOMPARE(pollBlock.hasEnded(), false);
    QCOMPARE(pollBlock.numAnswers(), 2);
    QCOMPARE(pollBlock.question(), u"test"_s);
    QCOMPARE(answer0.id, "option1"_L1);
    QCOMPARE(answer1.id, "option2"_L1);
    QCOMPARE(answer0.text, "option1text"_L1);
    QCOMPARE(answer1.text, "option2text"_L1);
    QCOMPARE(pollBlock.answerCountAtId(answer0.id), 0);
    QCOMPARE(pollBlock.answerCountAtId(answer1.id), 0);
    QCOMPARE(pollBlock.checkMemberSelectedId(connection->userId(), answer0.id), false);
    QCOMPARE(pollBlock.checkMemberSelectedId(connection->userId(), answer1.id), false);
    QCOMPARE(pollBlock.kind(), PollKind::Undisclosed);
}

QTEST_GUILESS_MAIN(PollBlockTest)
#include "pollblocktest.moc"
