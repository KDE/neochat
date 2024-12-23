// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QVariantList>

#include "chatbarcache.h"
#include "models/actionsmodel.h"

#include "testutils.h"

using namespace Quotient;

//TODO: rainbow, rainbowme, plain, spoiler, me, join, knock, j, part, leave, nick, roomnick, myroomnick, ignore, unignore, react, ban, unban, kick

class ActionsTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestUtils::TestRoom *room = nullptr;

    void expectMessage(const QString &actionName, const QString &args, MessageType::Type type, const QString &message);

private Q_SLOTS:
    void initTestCase();
    void testActions();
    void testActions_data();
    void testInvite();
};

void ActionsTest::initTestCase()
{
    connection = Connection::makeMockConnection(QStringLiteral("@bob:kde.org"));
    room = new TestUtils::TestRoom(connection, QStringLiteral("#myroom:kde.org"), QLatin1String("test-min-sync.json"));
}

void ActionsTest::testActions_data()
{
    QTest::addColumn<QString>("command");
    QTest::addColumn<std::optional<QString>>("resultText");
    QTest::addColumn<std::optional<Quotient::RoomMessageEvent::MsgType>>("type");

    QTest::newRow("shrug") << u"/shrug Hello"_s << std::make_optional(u"¯\\\\_(ツ)_/¯ Hello"_s)
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
    QSignalSpy spy(room, &TestUtils::TestRoom::showMessage);
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
    expectMessage(u"invite"_s, u"@example:example.org"_s, MessageType::Information, u"@example:example.org is already in this room."_s);
    QCOMPARE(room->memberState(u"@example:example.org"_s), Membership::Join);

    QCOMPARE(room->memberState(u"@user:example.com"_s), Membership::Leave);
    expectMessage(u"invite"_s, u"@user:example.com"_s, MessageType::Positive, u"@user:example.com was invited into this room."_s);

    //TODO mock server, wait for invite state to change
    //TODO QCOMPARE(room->memberState(u"@user:example.com"_s), Membership::Invite);
}

QTEST_MAIN(ActionsTest)
#include "actionstest.moc"
