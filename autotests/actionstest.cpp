// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QVariantList>

#include "accountmanager.h"
#include "chatbarcache.h"
#include "models/actionsmodel.h"

#include "server.h"
#include "testutils.h"

using namespace Quotient;

//TODO: rainbow, rainbowme, plain, spoiler, me, join, knock, j, part, leave, nick, roomnick, myroomnick, ignore, unignore, react, ban, unban, kick

class ActionsTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    NeoChatRoom *room = nullptr;

    void expectMessage(const QString &actionName, const QString &args, MessageType::Type type, const QString &message);

    Server server;

private Q_SLOTS:
    void initTestCase();
    void testActions();
    void testActions_data();
    void testInvite();
};

void ActionsTest::initTestCase()
{
    Connection::setRoomType<NeoChatRoom>();
    server.start();
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("neochat"));
    auto accountManager = new AccountManager(true);
    QSignalSpy spy(accountManager, &AccountManager::connectionAdded);
    connection = accountManager->accounts()->front();
    auto roomId = server.createRoom(u"@user:localhost:1234"_s);
    server.inviteUser(roomId, u"@invited:example.com"_s);
    server.banUser(roomId, u"@banned:example.com"_s);
    server.joinUser(roomId, u"@example:example.com"_s);

    QSignalSpy syncSpy(connection, &Connection::syncDone);
    // We need to wait for two syncs, as the next one won't have the changes yet
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
    room = dynamic_cast<NeoChatRoom *>(connection->room(roomId));
    QVERIFY(room);
}

void ActionsTest::testActions_data()
{
    QTest::addColumn<QString>("command");
    QTest::addColumn<std::optional<QString>>("resultText");
    QTest::addColumn<std::optional<Quotient::RoomMessageEvent::MsgType>>("type");

    QTest::newRow("shrug") << u"/shrug Hello"_s << std::make_optional(u"¯\\\\\\_(ツ)\\_/¯ Hello"_s)
                           << std::make_optional(Quotient::RoomMessageEvent::MsgType::Text);
    QTest::newRow("lenny") << u"/lenny Hello"_s << std::make_optional(u"( ͡° ͜ʖ ͡°) Hello"_s) << std::make_optional(Quotient::RoomMessageEvent::MsgType::Text);
    QTest::newRow("tableflip") << u"/tableflip Hello"_s << std::make_optional(u"(╯°□°）╯︵ ┻━┻ Hello"_s)
                               << std::make_optional(Quotient::RoomMessageEvent::MsgType::Text);
    QTest::newRow("unflip") << u"/unflip Hello"_s << std::make_optional(u"┬──┬ ノ( ゜-゜ノ) Hello"_s)
                            << std::make_optional(Quotient::RoomMessageEvent::MsgType::Text);
    QTest::newRow("rainbow") << u"/rainbow Hello"_s << std::optional<QString>() << std::optional<Quotient::RoomMessageEvent::MsgType>();
    QTest::newRow("rainbowme") << u"/rainbowme Hello"_s << std::optional<QString>() << std::optional<Quotient::RoomMessageEvent::MsgType>();
    QTest::newRow("plain") << u"/plain <b>Hello</b>"_s << std::optional<QString>() << std::optional<Quotient::RoomMessageEvent::MsgType>();
    QTest::newRow("spoiler") << u"/spoiler Hello"_s << std::optional<QString>() << std::optional<Quotient::RoomMessageEvent::MsgType>();
    QTest::newRow("me") << u"/me Hello"_s << std::make_optional(u"Hello"_s) << std::make_optional(Quotient::RoomMessageEvent::MsgType::Emote);
    QTest::newRow("notice") << u"/notice Hello"_s << std::make_optional(u"Hello"_s) << std::make_optional(Quotient::RoomMessageEvent::MsgType::Notice);
    QTest::newRow("message") << u"Hello"_s << std::make_optional(u"Hello"_s) << std::make_optional(Quotient::RoomMessageEvent::MsgType::Text);
    QTest::newRow("invite") << u"/invite @foo:example.com"_s << std::optional<QString>() << std::optional<Quotient::RoomMessageEvent::MsgType>();

    //TODO: join, knock, j, part, leave, nick, roomnick, myroomnick, ignore, unignore, react, ban, unban, kick
}

void ActionsTest::testActions()
{
    QFETCH(QString, command);
    QFETCH(std::optional<QString>, resultText);
    QFETCH(std::optional<Quotient::RoomMessageEvent::MsgType>, type);

    auto cache = new ChatBarCache();
    cache->setText(command);
    auto result = ActionsModel::handleAction(room, cache);
    QCOMPARE(resultText, std::get<std::optional<QString>>(result));
    QCOMPARE(type, std::get<std::optional<Quotient::RoomMessageEvent::MsgType>>(result));
}

static ActionsModel::Action findAction(const QString &name)
{
    for (const auto &action : ActionsModel::allActions()) {
        if (action.prefix == name) {
            return action;
        }
    }

    return {};
}

void ActionsTest::expectMessage(const QString &actionName, const QString &args, MessageType::Type type, const QString &message)
{
    auto action = findAction(actionName);
    QSignalSpy spy(room, &NeoChatRoom::showMessage);
    auto result = action.handle(args, room, nullptr);
    auto expected = QVariantList {type, message};
    auto signal = spy.takeFirst();
    QCOMPARE(signal, expected);
}

void ActionsTest::testInvite()
{
    expectMessage(u"invite"_s, u"foo"_s, MessageType::Error, u"'foo' does not look like a matrix id."_s);
    expectMessage(u"invite"_s, u"@invited:example.com"_s, MessageType::Information, u"@invited:example.com is already invited to this room."_s);
    QCOMPARE(room->memberState(u"@invited:example.com"_s), Membership::Invite);
    expectMessage(u"invite"_s, u"@banned:example.com"_s, MessageType::Information, u"@banned:example.com is banned from this room."_s);
    QCOMPARE(room->memberState(u"@banned:example.com"_s), Membership::Ban);
    expectMessage(u"invite"_s, connection->userId(), MessageType::Positive, u"You are already in this room."_s);
    QCOMPARE(room->memberState(connection->userId()), Membership::Join);
    expectMessage(u"invite"_s, u"@example:example.com"_s, MessageType::Information, u"@example:example.com is already in this room."_s);
    QCOMPARE(room->memberState(u"@example:example.com"_s), Membership::Join);

    QCOMPARE(room->memberState(u"@user:example.com"_s), Membership::Leave);
    expectMessage(u"invite"_s, u"@user:example.com"_s, MessageType::Positive, u"@user:example.com was invited into this room."_s);

    QSignalSpy spy(room, &NeoChatRoom::changed);
    QVERIFY(spy.wait());

    auto tries = 0;

    while (room->memberState(u"@user:example.com"_s) != Membership::Invite) {
        QVERIFY(spy.wait());
        tries += 1;
        if (tries > 3) {
            QVERIFY(false);
        }
    }

    QCOMPARE(room->memberState(u"@user:example.com"_s), Membership::Invite);
}

QTEST_MAIN(ActionsTest)
#include "actionstest.moc"
