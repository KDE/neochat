// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTest>

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
    void edit();
    void attachment();
};

void ChatBarCacheTest::initTestCase()
{
    connection = Connection::makeMockConnection(QStringLiteral("@bob:kde.org"));
    room = new TestUtils::TestRoom(connection, QStringLiteral("#myroom:kde.org"), QLatin1String("test-min-sync.json"));
}

void ChatBarCacheTest::empty()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(room));

    QCOMPARE(chatBarCache->text(), QString());
    QCOMPARE(chatBarCache->isReplying(), false);
    QCOMPARE(chatBarCache->replyId(), QString());
    QCOMPARE(chatBarCache->isEditing(), false);
    QCOMPARE(chatBarCache->editId(), QString());
    QCOMPARE(chatBarCache->relationUser(), room->getUser(nullptr));
    QCOMPARE(chatBarCache->relationMessage(), QString());
    QCOMPARE(chatBarCache->attachmentPath(), QString());
}

void ChatBarCacheTest::noRoom()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache());
    chatBarCache->setReplyId(QLatin1String("$153456789:example.org"));

    // These should return empty even though a reply ID has been set because the
    // ChatBarCache has no parent.

    QTest::ignoreMessage(QtWarningMsg, "ChatBarCache created with no parent, a NeoChatRoom must be set as the parent on creation.");
    QCOMPARE(chatBarCache->relationUser(), QVariantMap());

    QTest::ignoreMessage(QtWarningMsg, "ChatBarCache created with no parent, a NeoChatRoom must be set as the parent on creation.");
    QCOMPARE(chatBarCache->relationMessage(), QString());
}

void ChatBarCacheTest::badParent()
{
    QScopedPointer<QObject> badParent(new QObject());
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(badParent.get()));
    chatBarCache->setReplyId(QLatin1String("$153456789:example.org"));

    // These should return empty even though a reply ID has been set because the
    // ChatBarCache has no parent.

    QTest::ignoreMessage(QtWarningMsg, "ChatBarCache created with incorrect parent, a NeoChatRoom must be set as the parent on creation.");
    QCOMPARE(chatBarCache->relationUser(), QVariantMap());

    QTest::ignoreMessage(QtWarningMsg, "ChatBarCache created with incorrect parent, a NeoChatRoom must be set as the parent on creation.");
    QCOMPARE(chatBarCache->relationMessage(), QString());
}

void ChatBarCacheTest::reply()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(room));
    chatBarCache->setText(QLatin1String("some text"));
    chatBarCache->setAttachmentPath(QLatin1String("some/path"));
    chatBarCache->setReplyId(QLatin1String("$153456789:example.org"));

    QCOMPARE(chatBarCache->text(), QLatin1String("some text"));
    QCOMPARE(chatBarCache->isReplying(), true);
    QCOMPARE(chatBarCache->replyId(), QLatin1String("$153456789:example.org"));
    QCOMPARE(chatBarCache->isEditing(), false);
    QCOMPARE(chatBarCache->editId(), QString());
    QCOMPARE(chatBarCache->relationUser(), room->getUser(room->user(QLatin1String("@example:example.org"))));
    QCOMPARE(chatBarCache->relationMessage(), QLatin1String("This is an example\ntext message"));
    QCOMPARE(chatBarCache->attachmentPath(), QString());
}

void ChatBarCacheTest::edit()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(room));
    chatBarCache->setText(QLatin1String("some text"));
    chatBarCache->setAttachmentPath(QLatin1String("some/path"));
    chatBarCache->setEditId(QLatin1String("$153456789:example.org"));

    QCOMPARE(chatBarCache->text(), QLatin1String("some text"));
    QCOMPARE(chatBarCache->isReplying(), false);
    QCOMPARE(chatBarCache->replyId(), QString());
    QCOMPARE(chatBarCache->isEditing(), true);
    QCOMPARE(chatBarCache->editId(), QLatin1String("$153456789:example.org"));
    QCOMPARE(chatBarCache->relationUser(), room->getUser(room->user(QLatin1String("@example:example.org"))));
    QCOMPARE(chatBarCache->relationMessage(), QLatin1String("This is an example\ntext message"));
    QCOMPARE(chatBarCache->attachmentPath(), QString());
}

void ChatBarCacheTest::attachment()
{
    QScopedPointer<ChatBarCache> chatBarCache(new ChatBarCache(room));
    chatBarCache->setText(QLatin1String("some text"));
    chatBarCache->setEditId(QLatin1String("$153456789:example.org"));
    chatBarCache->setAttachmentPath(QLatin1String("some/path"));

    QCOMPARE(chatBarCache->text(), QLatin1String("some text"));
    QCOMPARE(chatBarCache->isReplying(), false);
    QCOMPARE(chatBarCache->replyId(), QString());
    QCOMPARE(chatBarCache->isEditing(), false);
    QCOMPARE(chatBarCache->editId(), QString());
    QCOMPARE(chatBarCache->relationUser(), room->getUser(nullptr));
    QCOMPARE(chatBarCache->relationMessage(), QString());
    QCOMPARE(chatBarCache->attachmentPath(), QLatin1String("some/path"));
}

QTEST_MAIN(ChatBarCacheTest)
#include "chatbarcachetest.moc"
