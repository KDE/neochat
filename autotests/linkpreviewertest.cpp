// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>

#include "linkpreviewer.h"

#include <Quotient/events/roommessageevent.h>
#include <Quotient/quotient_common.h>
#include <Quotient/syncdata.h>

#include "utils.h"

#include "testutils.h"

using namespace Quotient;

class LinkPreviewerTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;
    TestUtils::TestRoom *room = nullptr;

private Q_SLOTS:
    void initTestCase();

    void linkPreviewsMatch_data();
    void linkPreviewsMatch();

    void linkPreviewsReject_data();
    void linkPreviewsReject();

    void editedLink();
};

void LinkPreviewerTest::initTestCase()
{
    connection = Connection::makeMockConnection(QStringLiteral("@bob:example.org"));
    room = new TestUtils::TestRoom(connection, QStringLiteral("!test:example.org"));
}

void LinkPreviewerTest::linkPreviewsMatch_data()
{
    QTest::addColumn<QString>("eventSource");
    QTest::addColumn<QUrl>("testOutputLink");

    QTest::newRow("plainHttps") << QStringLiteral("test-validplainlink-event.json") << QUrl("https://kde.org"_ls);
    QTest::newRow("richHttps") << QStringLiteral("test-validrichlink-event.json") << QUrl("https://kde.org"_ls);
    QTest::newRow("plainWww") << QStringLiteral("test-validplainwwwlink-event.json") << QUrl("www.example.org"_ls);
    QTest::newRow("multipleHttps") << QStringLiteral("test-multiplelink-event.json") << QUrl("www.example.org"_ls);
}

void LinkPreviewerTest::linkPreviewsMatch()
{
    QFETCH(QString, eventSource);
    QFETCH(QUrl, testOutputLink);

    auto event = TestUtils::loadEventFromFile<RoomMessageEvent>(eventSource);
    auto linkPreviewer = LinkPreviewer(room, event.get());

    QCOMPARE(linkPreviewer.empty(), false);
    QCOMPARE(linkPreviewer.url(), testOutputLink);
}

void LinkPreviewerTest::linkPreviewsReject_data()
{
    QTest::addColumn<QString>("eventSource");

    QTest::newRow("mxc") << QStringLiteral("test-invalidmxclink-event.json");
    QTest::newRow("matrixTo") << QStringLiteral("test-invalidmatrixtolink-event.json");
    QTest::newRow("noSpace") << QStringLiteral("test-invalidnospacelink-event.json");
}

void LinkPreviewerTest::linkPreviewsReject()
{
    QFETCH(QString, eventSource);

    auto event = TestUtils::loadEventFromFile<RoomMessageEvent>(eventSource);
    auto linkPreviewer = LinkPreviewer(room, event.get());

    QCOMPARE(linkPreviewer.empty(), true);
    QCOMPARE(linkPreviewer.url(), QUrl());
}

void LinkPreviewerTest::editedLink()
{
    room->syncNewEvents(QStringLiteral("test-linkpreviewerintial-sync.json"));
    auto event = eventCast<const RoomMessageEvent>(room->messageEvents().at(0).get());
    auto linkPreviewer = LinkPreviewer(room, event);

    QCOMPARE(linkPreviewer.empty(), false);
    QCOMPARE(linkPreviewer.url(), QUrl("https://kde.org"_ls));

    room->syncNewEvents(QStringLiteral("test-linkpreviewerreplace-sync.json"));

    QCOMPARE(linkPreviewer.empty(), true);
    QCOMPARE(linkPreviewer.url(), QUrl());
}

QTEST_MAIN(LinkPreviewerTest)
#include "linkpreviewertest.moc"
