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

#include <KLocalizedString>

#include "accountmanager.h"
#include "blockcache.h"
#include "chatbarcache.h"
#include "neochatroom.h"

#include "server.h"
#include "testutils.h"

using namespace Quotient;

class ChatBarCacheTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    NeoChatRoom *room = nullptr;
    Server server;
    QString eventId;

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
    Connection::setRoomType<NeoChatRoom>();
    server.start();
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("neochat"));
    auto accountManager = new AccountManager(true, this);
    QSignalSpy spy(accountManager, &AccountManager::connectionAdded);
    connection = dynamic_cast<NeoChatConnection *>(accountManager->accounts()->front());

    const auto roomId = server.createRoom(u"@user:localhost:1234"_s);
    eventId = server.sendEvent(roomId,
                               u"m.room.message"_s,
                               QJsonObject{
                                   {u"body"_s, u"foo"_s},
                                   {u"msgtype"_s, u"m.text"_s},
                               });

    QSignalSpy syncSpy(connection, &Connection::syncDone);
    // We need to wait for two syncs, as the next one won't have the changes yet
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
    room = dynamic_cast<NeoChatRoom *>(connection->room(roomId));
    QVERIFY(room);

    server.joinUser(room->id(), u"@foo:server.com"_s);
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
}

void ChatBarCacheTest::empty()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(room));

    QCOMPARE(chatBarCache->cache().toString(), QString());
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
    QTest::ignoreMessage(QtWarningMsg, "ChatBarCache created with no parent, a NeoChatRoom must be set as the parent on creation.");
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache());
    chatBarCache->setReplyId(eventId);

    // These should return empty even though a reply ID has been set because the
    // ChatBarCache has no parent.

    QTest::ignoreMessage(QtWarningMsg, "ChatBarCache created with no parent, a NeoChatRoom must be set as the parent on creation.");
    QCOMPARE(chatBarCache->relationAuthor(), Quotient::RoomMember());

    QTest::ignoreMessage(QtWarningMsg, "ChatBarCache created with no parent, a NeoChatRoom must be set as the parent on creation.");
    QCOMPARE(chatBarCache->relationMessage(), QString());
}

void ChatBarCacheTest::badParent()
{
    QTest::ignoreMessage(QtWarningMsg, "ChatBarCache created with incorrect parent, a NeoChatRoom must be set as the parent on creation.");
    QScopedPointer<QObject> badParent(new QObject());
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(badParent.get()));
    chatBarCache->setReplyId(eventId);

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
    chatBarCache->cache() += Block::CacheItem{.type = MessageComponentType::Text, .content = QTextDocumentFragment::fromMarkdown(u"some text"_s)};
    chatBarCache->setAttachmentPath(u"some/path"_s);
    chatBarCache->setReplyId(eventId);

    QCOMPARE(chatBarCache->cache().toString(), u"some text"_s);
    QCOMPARE(chatBarCache->isReplying(), true);
    QCOMPARE(chatBarCache->replyId(), eventId);
    QCOMPARE(chatBarCache->isEditing(), false);
    QCOMPARE(chatBarCache->editId(), QString());
    QCOMPARE(chatBarCache->relationAuthor(), room->member(u"@foo:server.com"_s));
    QCOMPARE(chatBarCache->relationMessage(), u"foo"_s);
    QCOMPARE(chatBarCache->attachmentPath(), QString());
    QCOMPARE(chatBarCache->relationAuthorIsPresent(), true);
}

void ChatBarCacheTest::replyMissingUser()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(room));
    chatBarCache->cache() += Block::CacheItem{.type = MessageComponentType::Text, .content = QTextDocumentFragment::fromMarkdown(u"some text"_s)};
    chatBarCache->setAttachmentPath(u"some/path"_s);
    chatBarCache->setReplyId(eventId);

    QCOMPARE(chatBarCache->cache().toString(), u"some text"_s);
    QCOMPARE(chatBarCache->isReplying(), true);
    QCOMPARE(chatBarCache->replyId(), eventId);
    QCOMPARE(chatBarCache->isEditing(), false);
    QCOMPARE(chatBarCache->editId(), QString());
    QCOMPARE(chatBarCache->relationAuthor(), room->member(u"@foo:server.com"_s));
    QCOMPARE(chatBarCache->relationMessage(), u"foo"_s);
    QCOMPARE(chatBarCache->attachmentPath(), QString());
    QCOMPARE(chatBarCache->relationAuthorIsPresent(), true);

    QSignalSpy relationAuthorIsPresentSpy(chatBarCache.get(), &ChatBarCache::relationAuthorIsPresentChanged);

    // sync again, which will simulate the reply user leaving the room

    QSignalSpy syncSpy(connection, &Connection::syncDone);
    server.sendStateEvent(room->id(), u"m.room.member"_s, u"@foo:server.com"_s, {{u"membership"_s, u"leave"_s}});
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());

    QTRY_COMPARE(relationAuthorIsPresentSpy.count(), 1);
    QCOMPARE(chatBarCache->relationAuthorIsPresent(), false);
}

void ChatBarCacheTest::edit()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(room));

    chatBarCache->cache() += Block::CacheItem{.type = MessageComponentType::Text, .content = QTextDocumentFragment::fromMarkdown(u"some text"_s)};
    chatBarCache->setAttachmentPath(u"some/path"_s);
    connect(chatBarCache.get(), &ChatBarCache::relationIdChanged, this, [this](const QString &oldEventId, const QString &newEventId) {
        QCOMPARE(oldEventId, QString());
        QCOMPARE(newEventId, eventId);
    });
    chatBarCache->setEditId(eventId);

    QCOMPARE(chatBarCache->cache().toString(), u"some text"_s);
    QCOMPARE(chatBarCache->isReplying(), false);
    QCOMPARE(chatBarCache->replyId(), QString());
    QCOMPARE(chatBarCache->isEditing(), true);
    QCOMPARE(chatBarCache->editId(), eventId);
    QCOMPARE(chatBarCache->relationAuthor(), room->member(u"@foo:server.com"_s));
    QCOMPARE(chatBarCache->relationMessage(), u"foo"_s);
    QCOMPARE(chatBarCache->attachmentPath(), QString());
}

void ChatBarCacheTest::attachment()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(room));
    chatBarCache->cache() += Block::CacheItem{.type = MessageComponentType::Text, .content = QTextDocumentFragment::fromMarkdown(u"some text"_s)};
    chatBarCache->setEditId(eventId);
    chatBarCache->setAttachmentPath(u"some/path"_s);

    QCOMPARE(chatBarCache->cache().toString(), u"some text"_s);
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
