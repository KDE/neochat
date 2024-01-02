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
    connection = Connection::makeMockConnection(QStringLiteral("@bob:kde.org"));
    room = new TestUtils::TestRoom(connection, QStringLiteral("#myroom:kde.org"), "test-pollhandlerstart-sync.json"_ls);
}

// Basically don't crash.
void PollHandlerTest::nullObject()
{
    auto pollHandler = PollHandler();

    QCOMPARE(pollHandler.hasEnded(), false);
    QCOMPARE(pollHandler.answerCount(), 0);
    QCOMPARE(pollHandler.question(), QString());
    QCOMPARE(pollHandler.options(), QJsonArray());
    QCOMPARE(pollHandler.answers(), QJsonObject());
    QCOMPARE(pollHandler.counts(), QJsonObject());
    QCOMPARE(pollHandler.kind(), QString());
}

void PollHandlerTest::poll()
{
    auto startEvent = eventCast<const PollStartEvent>(room->messageEvents().at(0).get());
    auto pollHandler = PollHandler(room, startEvent);

    auto options = QJsonArray{QJsonObject{{"id"_ls, "option1"_ls}, {"org.matrix.msc1767.text"_ls, "option1"_ls}},
                              QJsonObject{{"id"_ls, "option2"_ls}, {"org.matrix.msc1767.text"_ls, "option2"_ls}}};

    QCOMPARE(pollHandler.hasEnded(), false);
    QCOMPARE(pollHandler.answerCount(), 0);
    QCOMPARE(pollHandler.question(), QStringLiteral("test"));
    QCOMPARE(pollHandler.options(), options);
    QCOMPARE(pollHandler.answers(), QJsonObject());
    QCOMPARE(pollHandler.counts(), QJsonObject());
    QCOMPARE(pollHandler.kind(), QStringLiteral("org.matrix.msc3381.poll.disclosed"));
}

QTEST_GUILESS_MAIN(PollHandlerTest)
#include "pollhandlertest.moc"
