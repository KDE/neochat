// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <Quotient/connection.h>
#include <Quotient/quotient_common.h>
#include <Quotient/syncdata.h>

#include "events/pollevent.h"
#include "pollhandler.h"
#include "testutils.h"

using namespace Quotient;

class PollHandlerTest : public QObject
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

void PollHandlerTest::initTestCase()
{
    connection = Connection::makeMockConnection(u"@bob:kde.org"_s);
    room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s, u"test-pollhandlerstart-sync.json"_s);
}

// Basically don't crash.
void PollHandlerTest::nullObject()
{
    auto pollHandler = PollHandler();

    QCOMPARE(pollHandler.hasEnded(), false);
    QCOMPARE(pollHandler.numAnswers(), 0);
    QCOMPARE(pollHandler.question(), QString());
    QCOMPARE(pollHandler.kind(), PollKind::Disclosed);
}

void PollHandlerTest::poll()
{
    auto startEvent = eventCast<const PollStartEvent>(room->messageEvents().at(0).get());
    auto pollHandler = PollHandler(room, startEvent->id());

    QList<Quotient::EventContent::Answer> options = {EventContent::Answer{"option1"_L1, "option1"_L1}, EventContent::Answer{"option2"_L1, "option2"_L1}};

    const auto answer0 = pollHandler.answerAtRow(0);
    const auto answer1 = pollHandler.answerAtRow(1);
    QCOMPARE(pollHandler.hasEnded(), false);
    QCOMPARE(pollHandler.numAnswers(), 2);
    QCOMPARE(pollHandler.question(), u"test"_s);
    QCOMPARE(answer0.id, "option1"_L1);
    QCOMPARE(answer1.id, "option2"_L1);
    QCOMPARE(answer0.text, "option1text"_L1);
    QCOMPARE(answer1.text, "option2text"_L1);
    QCOMPARE(pollHandler.answerCountAtId(answer0.id), 0);
    QCOMPARE(pollHandler.answerCountAtId(answer1.id), 0);
    QCOMPARE(pollHandler.checkMemberSelectedId(connection->userId(), answer0.id), false);
    QCOMPARE(pollHandler.checkMemberSelectedId(connection->userId(), answer1.id), false);
    QCOMPARE(pollHandler.kind(), PollKind::Undisclosed);
}

QTEST_GUILESS_MAIN(PollHandlerTest)
#include "pollhandlertest.moc"
