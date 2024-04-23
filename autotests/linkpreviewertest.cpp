// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QTest>

#include "linkpreviewer.h"

#include "utils.h"
#include <Quotient/events/roommessageevent.h>
#include <Quotient/quotient_common.h>
#include <Quotient/syncdata.h>

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

    void multipleLinkPreviewsMatch_data();
    void multipleLinkPreviewsMatch();

    void linkPreviewsReject_data();
    void linkPreviewsReject();
};

void LinkPreviewerTest::initTestCase()
{
    connection = Connection::makeMockConnection(QStringLiteral("@bob:example.org"));
    room = new TestUtils::TestRoom(connection, QStringLiteral("!test:example.org"));
}

void LinkPreviewerTest::linkPreviewsMatch_data()
{
    QTest::addColumn<QString>("inputString");
    QTest::addColumn<QUrl>("testOutputLink");

    QTest::newRow("plainHttps") << QStringLiteral("https://kde.org") << QUrl("https://kde.org"_ls);
    QTest::newRow("richHttps") << QStringLiteral("<a href=\"https://kde.org\">Rich Link</a>") << QUrl("https://kde.org"_ls);
    QTest::newRow("richHttpsLinkDescription") << QStringLiteral("<a href=\"https://kde.org\">https://kde.org</a>") << QUrl("https://kde.org"_ls);
}

void LinkPreviewerTest::linkPreviewsMatch()
{
    QFETCH(QString, inputString);
    QFETCH(QUrl, testOutputLink);

    auto link = LinkPreviewer::linkPreviews(inputString)[0];

    QCOMPARE(link, testOutputLink);
}

void LinkPreviewerTest::multipleLinkPreviewsMatch_data()
{
    QTest::addColumn<QString>("inputString");
    QTest::addColumn<QList<QUrl>>("testOutputLinks");

    QTest::newRow("multipleHttps") << QStringLiteral("www.example.org https://kde.org") << QList{QUrl("www.example.org"_ls), QUrl("https://kde.org"_ls)};
    QTest::newRow("multipleHttps1Invalid") << QStringLiteral("www.example.org mxc://example.org/SEsfnsuifSDFSSEF") << QList{QUrl("www.example.org"_ls)};
}

void LinkPreviewerTest::multipleLinkPreviewsMatch()
{
    QFETCH(QString, inputString);
    QFETCH(QList<QUrl>, testOutputLinks);

    auto links = LinkPreviewer::linkPreviews(inputString);

    QCOMPARE(links, testOutputLinks);
}

void LinkPreviewerTest::linkPreviewsReject_data()
{
    QTest::addColumn<QString>("inputString");

    QTest::newRow("mxc") << QStringLiteral("mxc://example.org/SEsfnsuifSDFSSEF");
    QTest::newRow("matrixTo") << QStringLiteral("https://matrix.to/#/@alice:example.org");
    QTest::newRow("noSpace") << QStringLiteral("testhttps://kde.org");
}

void LinkPreviewerTest::linkPreviewsReject()
{
    QFETCH(QString, inputString);

    auto links = LinkPreviewer::linkPreviews(inputString);

    QCOMPARE(links.empty(), true);
}

QTEST_MAIN(LinkPreviewerTest)
#include "linkpreviewertest.moc"
