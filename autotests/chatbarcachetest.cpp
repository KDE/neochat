// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTest>

#include <QSignalSpy>
#include <Quotient/roommember.h>
#include <Quotient/syncdata.h>
#include <qtestcase.h>

#include "chatbarcache.h"
#include "neochatroom.h"

#include "testutils.h"

using namespace Quotient;

class ChatBarCacheTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestUtils::TestRoom *room = nullptr;

private Q_SLOTS:
    void initTestCase();

    void empty();
    void noRoom();
    void badParent();
    void reply();
    void replyMissingUser();
    void edit();
    void attachment();
};

void ChatBarCacheTest::initTestCase()
{
    connection = Connection::makeMockConnection(u"@bob:kde.org"_s);
    room = new TestUtils::TestRoom(connection, u"#myroom:kde.org"_s, "test-min-sync.json"_L1);
}

void ChatBarCacheTest::empty()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(room));

    QCOMPARE(chatBarCache->text(), QString());
    QCOMPARE(chatBarCache->isReplying(), false);
    QCOMPARE(chatBarCache->replyId(), QString());
    QCOMPARE(chatBarCache->isEditing(), false);
    QCOMPARE(chatBarCache->editId(), QString());
    QCOMPARE(chatBarCache->relationAuthor(), room->member(QString()));
    QCOMPARE(chatBarCache->relationMessage(), QString());
    QCOMPARE(chatBarCache->attachmentPath(), QString());
}

void ChatBarCacheTest::noRoom()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache());
    chatBarCache->setReplyId(u"$153456789:example.org"_s);

    // These should return empty even though a reply ID has been set because the
    // ChatBarCache has no parent.

    QTest::ignoreMessage(QtWarningMsg, "ChatBarCache created with no parent, a NeoChatRoom must be set as the parent on creation.");
    QCOMPARE(chatBarCache->relationAuthor(), Quotient::RoomMember());

    QTest::ignoreMessage(QtWarningMsg, "ChatBarCache created with no parent, a NeoChatRoom must be set as the parent on creation.");
    QCOMPARE(chatBarCache->relationMessage(), QString());
}

void ChatBarCacheTest::badParent()
{
    QScopedPointer<QObject> badParent(new QObject());
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(badParent.get()));
    chatBarCache->setReplyId(u"$153456789:example.org"_s);

    // These should return empty even though a reply ID has been set because the
    // ChatBarCache has no parent.

    QTest::ignoreMessage(QtWarningMsg, "ChatBarCache created with incorrect parent, a NeoChatRoom must be set as the parent on creation.");
    QCOMPARE(chatBarCache->relationAuthor(), Quotient::RoomMember());

    QTest::ignoreMessage(QtWarningMsg, "ChatBarCache created with incorrect parent, a NeoChatRoom must be set as the parent on creation.");
    QCOMPARE(chatBarCache->relationMessage(), QString());
}

void ChatBarCacheTest::reply()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(room));
    chatBarCache->setText(u"some text"_s);
    chatBarCache->setAttachmentPath(u"some/path"_s);
    chatBarCache->setReplyId(u"$153456789:example.org"_s);

    QCOMPARE(chatBarCache->text(), u"some text"_s);
    QCOMPARE(chatBarCache->isReplying(), true);
    QCOMPARE(chatBarCache->replyId(), u"$153456789:example.org"_s);
    QCOMPARE(chatBarCache->isEditing(), false);
    QCOMPARE(chatBarCache->editId(), QString());
    QCOMPARE(chatBarCache->relationAuthor(), room->member(u"@example:example.org"_s));
    QCOMPARE(chatBarCache->relationMessage(), u"This is an example\ntext message"_s);
    QCOMPARE(chatBarCache->attachmentPath(), QString());
    QCOMPARE(chatBarCache->relationAuthorIsPresent(), true);
}

void ChatBarCacheTest::replyMissingUser()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(room));
    chatBarCache->setText(u"some text"_s);
    chatBarCache->setAttachmentPath(u"some/path"_s);
    chatBarCache->setReplyId(u"$153456789:example.org"_s);

    QCOMPARE(chatBarCache->text(), u"some text"_s);
    QCOMPARE(chatBarCache->isReplying(), true);
    QCOMPARE(chatBarCache->replyId(), u"$153456789:example.org"_s);
    QCOMPARE(chatBarCache->isEditing(), false);
    QCOMPARE(chatBarCache->editId(), QString());
    QCOMPARE(chatBarCache->relationAuthor(), room->member(u"@example:example.org"_s));
    QCOMPARE(chatBarCache->relationMessage(), u"This is an example\ntext message"_s);
    QCOMPARE(chatBarCache->attachmentPath(), QString());
    QCOMPARE(chatBarCache->relationAuthorIsPresent(), true);

    QSignalSpy relationAuthorIsPresentSpy(chatBarCache.get(), &ChatBarCache::relationAuthorIsPresentChanged);

    // sync again, which will simulate the reply user leaving the room
    room->syncNewEvents(u"test-min-sync-extra-sync.json"_s);

    QTRY_COMPARE(relationAuthorIsPresentSpy.count(), 1);
    QCOMPARE(chatBarCache->relationAuthorIsPresent(), false);
}

void ChatBarCacheTest::edit()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(room));

    chatBarCache->setText(u"some text"_s);
    chatBarCache->setAttachmentPath(u"some/path"_s);
    connect(chatBarCache.get(), &ChatBarCache::relationIdChanged, this, [](const QString &oldEventId, const QString &newEventId) {
        QCOMPARE(oldEventId, QString());
        QCOMPARE(newEventId, QString(u"$153456789:example.org"_s));
    });
    chatBarCache->setEditId(u"$153456789:example.org"_s);

    QCOMPARE(chatBarCache->text(), u"some text"_s);
    QCOMPARE(chatBarCache->isReplying(), false);
    QCOMPARE(chatBarCache->replyId(), QString());
    QCOMPARE(chatBarCache->isEditing(), true);
    QCOMPARE(chatBarCache->editId(), u"$153456789:example.org"_s);
    QCOMPARE(chatBarCache->relationAuthor(), room->member(u"@example:example.org"_s));
    QCOMPARE(chatBarCache->relationMessage(), u"This is an example\ntext message"_s);
    QCOMPARE(chatBarCache->attachmentPath(), QString());
}

void ChatBarCacheTest::attachment()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(room));
    chatBarCache->setText(u"some text"_s);
    chatBarCache->setEditId(u"$153456789:example.org"_s);
    chatBarCache->setAttachmentPath(u"some/path"_s);

    QCOMPARE(chatBarCache->text(), u"some text"_s);
    QCOMPARE(chatBarCache->isReplying(), false);
    QCOMPARE(chatBarCache->replyId(), QString());
    QCOMPARE(chatBarCache->isEditing(), false);
    QCOMPARE(chatBarCache->editId(), QString());
    QCOMPARE(chatBarCache->relationAuthor(), room->member(QString()));
    QCOMPARE(chatBarCache->relationMessage(), QString());
    QCOMPARE(chatBarCache->attachmentPath(), u"some/path"_s);
}

QTEST_MAIN(ChatBarCacheTest)
#include "chatbarcachetest.moc"
