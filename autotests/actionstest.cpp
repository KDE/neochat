// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>

#include "chatbarcache.h"
#include "models/actionsmodel.h"

#include "testutils.h"

using namespace Quotient;

class ActionsTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestUtils::TestRoom *room = nullptr;

private Q_SLOTS:
    void initTestCase();
    void testActions();
    void testActions_data();
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

QTEST_MAIN(ActionsTest)
#include "actionstest.moc"
