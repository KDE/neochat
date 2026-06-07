// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "models/chatbarmessagecontentmodel.h"

#include <QObject>
#include <QSignalSpy>

#include <Quotient/connection.h>

#include "block.h"
#include "enums/blocktype.h"
#include "neochatconnection.h"
#include "testutils.h"

using namespace Quotient;
using namespace Qt::Literals::StringLiterals;

class ChatBarMessageContentModelTest : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<Connection> connection = nullptr;
    std::unique_ptr<TestUtils::TestRoom> room = nullptr;

    void checkEmptyChatbar(const ChatBarMessageContentModel &model);

private Q_SLOTS:
    void initTestCase();

    void missingEvent();
    void addLocationTest();
};

void ChatBarMessageContentModelTest::checkEmptyChatbar(const ChatBarMessageContentModel &model)
{
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0), MessageContentModel::ComponentTypeRole), Blocks::Text);
    QCOMPARE(model.data(model.index(0), MessageContentModel::BlockRole).value<Blocks::TextBlock *>()->hasSpoiler(), false);
}

void ChatBarMessageContentModelTest::initTestCase()
{
    connection = std::make_unique<NeoChatConnection>();
    room = std::make_unique<TestUtils::TestRoom>(connection.get(), u"#firstRoom:kde.org"_s);
}

void ChatBarMessageContentModelTest::missingEvent()
{
    auto model = ChatBarMessageContentModel(this);
    model.setRoom(room.get());

    // Ensure we have a text block that's initialized correctly:
    checkEmptyChatbar(model);
}

void ChatBarMessageContentModelTest::addLocationTest()
{
    auto model = ChatBarMessageContentModel(this);
    model.setRoom(room.get());

    model.addLocation(51.606, 0.046, u"m.pin"_s);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.data(model.index(0), MessageContentModel::ComponentTypeRole), Blocks::Location);
    QCOMPARE(model.data(model.index(0), MessageContentModel::BlockRole).value<Blocks::LocationBlock *>()->latitude(), 51.606);
    QCOMPARE(model.data(model.index(0), MessageContentModel::BlockRole).value<Blocks::LocationBlock *>()->longitude(), 0.046);
    QCOMPARE(model.data(model.index(0), MessageContentModel::BlockRole).value<Blocks::LocationBlock *>()->asset(), u"m.pin"_s);
    QCOMPARE(model.data(model.index(1), MessageContentModel::ComponentTypeRole), Blocks::Text);
    QCOMPARE(model.data(model.index(1), MessageContentModel::BlockRole).value<Blocks::TextBlock *>()->item()->initialFragment().toPlainText(), u"User's pin"_s);

    model.removeAttachment();
    checkEmptyChatbar(model);
}

QTEST_MAIN(ChatBarMessageContentModelTest)

#include "chatbarmessagecontentmodeltest.moc"
