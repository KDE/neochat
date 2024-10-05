// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QTest>

#include "actionshandler.h"
#include "chatbarcache.h"

#include "testutils.h"

class ActionsHandlerTest : public QObject
{
    Q_OBJECT

private:
    Quotient::Connection *connection = Quotient::Connection::makeMockConnection(QStringLiteral("@bob:kde.org"));

private Q_SLOTS:
    void nullObject();
};

void ActionsHandlerTest::nullObject()
{
    QTest::ignoreMessage(QtWarningMsg, "ActionsHandler::handleMessageEvent - called with m_room and/or chatBarCache set to nullptr.");
    ActionsHandler::handleMessageEvent(nullptr, nullptr);

    auto chatBarCache = new ChatBarCache(this);
    QTest::ignoreMessage(QtWarningMsg, "ActionsHandler::handleMessageEvent - called with m_room and/or chatBarCache set to nullptr.");
    ActionsHandler::handleMessageEvent(nullptr, chatBarCache);

    auto room = new TestUtils::TestRoom(connection, QStringLiteral("#myroom:kde.org"));
    QTest::ignoreMessage(QtWarningMsg, "ActionsHandler::handleMessageEvent - called with m_room and/or chatBarCache set to nullptr.");
    ActionsHandler::handleMessageEvent(room, nullptr);

    // The final one should throw no warning so we make sure.
    QTest::failOnWarning("ActionsHandler::handleMessageEvent - called with m_room and/or chatBarCache set to nullptr.");
    ActionsHandler::handleMessageEvent(room, chatBarCache);
}

QTEST_GUILESS_MAIN(ActionsHandlerTest)
#include "actionshandlertest.moc"
