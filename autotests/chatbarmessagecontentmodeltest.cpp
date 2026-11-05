// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "models/chatbarmessagecontentmodel.h"

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
    void removeAttachmentTest();
    void keepSentAttachmentTest();
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

void ChatBarMessageContentModelTest::removeAttachmentTest()
{
    auto model = ChatBarMessageContentModel(this);
    model.addAttachment(QUrl::fromLocalFile(QString::fromUtf8(__FILE__)));
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.data(model.index(0), MessageContentModel::ComponentTypeRole), Blocks::File);

    model.removeAttachment();
    checkEmptyChatbar(model);
}

void ChatBarMessageContentModelTest::keepSentAttachmentTest()
{
    // When editing a media message, the chat bar shows the attachment that has already been sent.
    Blocks::Cache cache;
    cache.append(std::make_unique<Blocks::ImageCacheItem>(Blocks::Image, QUrl(u"mxc://example.org/JWEIFJgwEIhweiWJE"_s), u"cat.png"_s, Blocks::ImageInfo{}));
    cache.append(std::make_unique<Blocks::TextCacheItem>(Blocks::Text, QTextDocumentFragment::fromPlainText(u"A picture of a cat"_s)));

    auto model = ChatBarMessageContentModel(this);
    model.setCache(&cache);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.data(model.index(0), MessageContentModel::ComponentTypeRole), Blocks::Image);

    // It belongs to the message being edited, so it can't be removed.
    model.removeAttachment();
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.data(model.index(0), MessageContentModel::ComponentTypeRole), Blocks::Image);
    QVERIFY(model.hasAttachment());
}

QTEST_MAIN(ChatBarMessageContentModelTest)

#include "chatbarmessagecontentmodeltest.moc"
