// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "models/chatbarmessagecontentmodel.h"

#include <memory>

#include <QObject>
#include <QSignalSpy>

#include <Quotient/connection.h>

#include "block.h"
#include "blockcache.h"
#include "enums/blocktype.h"
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
    std::unique_ptr<TestUtils::TestRoom> room = nullptr;

    void checkEmptyChatbar(const ChatBarMessageContentModel &model);

private Q_SLOTS:
    void initTestCase();

    void missingEvent();
    void addLocationTest();
    void addAttachmentToReply();
    void unattachedTextIsEmpty();
    void clearUpdatesCache_data();
    void clearUpdatesCache();
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
    room = std::make_unique<TestUtils::TestRoom>(connection.get(), u"#firstRoom:kde.org"_s, u"test-messageventmodel-sync.json"_s);
}

void ChatBarMessageContentModelTest::missingEvent()
{
    auto model = ChatBarMessageContentModel(this);
    // Ensure we have a text block that's initialized correctly:
    checkEmptyChatbar(model);
}

void ChatBarMessageContentModelTest::addLocationTest()
{
    auto model = ChatBarMessageContentModel(this);
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

void ChatBarMessageContentModelTest::addAttachmentToReply()
{
    auto model = ChatBarMessageContentModel(this);
    QCOMPARE(model.rowCount(), 1);
    QVERIFY(room->timelineSize() > 0);
    model.addReply(ReplyModelHelper().modelForEvent(room.get(), room->messageEvents()[0].event()->id()));
    QCOMPARE(model.rowCount(), 2);
    model.addAttachment(QUrl(QString::fromUtf8(__FILE__)));
    QCOMPARE(model.rowCount(), 3);
}

void ChatBarMessageContentModelTest::unattachedTextIsEmpty()
{
    // No delegate has attached an editor yet, so emptiness must come from the initial content.
    ChatBarMessageContentModel model;
    QVERIFY(!model.hasAnyContent());

    Blocks::TextBlock emptyBlock(Blocks::Text, QTextDocumentFragment::fromPlainText(QString()), false, nullptr);
    QVERIFY(emptyBlock.isEmpty());

    Blocks::TextBlock filledBlock(Blocks::Text, QTextDocumentFragment::fromPlainText(u"pasted"_s), false, nullptr);
    QVERIFY(!filledBlock.isEmpty());
}

void ChatBarMessageContentModelTest::clearUpdatesCache_data()
{
    QTest::addColumn<bool>("resetModel");
    QTest::newRow("clear") << false;
    QTest::newRow("reset") << true;
}

void ChatBarMessageContentModelTest::clearUpdatesCache()
{
    QFETCH(bool, resetModel);

    Blocks::Cache cache;
    cache.append(std::make_unique<Blocks::TextCacheItem>(Blocks::Text, QTextDocumentFragment::fromPlainText(u"previous draft"_s)));
    ChatBarMessageContentModel model;
    model.setCache(&cache);
    QCOMPARE(cache.toPlainString(), u"previous draft"_s);

    if (resetModel) {
        model.resetModel();
    } else {
        model.clear();
    }

    checkEmptyChatbar(model);
    QVERIFY(cache.toPlainString().isEmpty());
    QVERIFY(cache.toString().isEmpty());
}

QTEST_MAIN(ChatBarMessageContentModelTest)

#include "chatbarmessagecontentmodeltest.moc"
