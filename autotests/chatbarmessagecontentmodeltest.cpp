// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "models/chatbarmessagecontentmodel.h"

#include <QObject>
#include <QSignalSpy>

#include <Quotient/connection.h>

#include "block.h"
#include "models/eventmessagecontentmodel.h"

#include "neochatconnection.h"
#include "testutils.h"

using namespace Quotient;
using namespace Qt::Literals::StringLiterals;

class ChatBarMessageContentModelTest : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<Connection> connection = nullptr;

private Q_SLOTS:
    void initTestCase();

    void missingEvent();
};

void ChatBarMessageContentModelTest::initTestCase()
{
    connection = std::make_unique<NeoChatConnection>();
}

void ChatBarMessageContentModelTest::missingEvent()
{
    auto room = std::make_unique<TestUtils::TestRoom>(connection.get(), u"#firstRoom:kde.org"_s);
    auto model1 = ChatBarMessageContentModel(this);
    model1.setRoom(room.get());

    // Ensure we have a text block that's initialized correctly:
    QCOMPARE(model1.rowCount(), 1);
    QCOMPARE(model1.data(model1.index(0), MessageContentModel::ComponentTypeRole), Blocks::Text);
    QCOMPARE(model1.data(model1.index(0), MessageContentModel::BlockRole).value<Blocks::TextBlock *>()->hasSpoiler(), false);
}

QTEST_MAIN(ChatBarMessageContentModelTest)

#include "chatbarmessagecontentmodeltest.moc"
