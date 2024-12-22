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
    connection = Connection::makeMockConnection(u"@bob:example.org"_s);
    room = new TestUtils::TestRoom(connection, u"!test:example.org"_s);
}

void LinkPreviewerTest::linkPreviewsMatch_data()
{
    QTest::addColumn<QString>("inputString");
    QTest::addColumn<QUrl>("testOutputLink");

    QTest::newRow("plainHttps") << u"https://kde.org"_s << QUrl(u"https://kde.org"_s);
    QTest::newRow("richHttps") << u"<a href=\"https://kde.org\">Rich Link</a>"_s << QUrl(u"https://kde.org"_s);
    QTest::newRow("richHttpsLinkDescription") << u"<a href=\"https://kde.org\">https://kde.org</a>"_s << QUrl(u"https://kde.org"_s);
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

    QTest::newRow("multipleHttps") << u"www.example.org https://kde.org"_s << QList{QUrl(u"www.example.org"_s), QUrl(u"https://kde.org"_s)};
    QTest::newRow("multipleHttps1Invalid") << u"www.example.org mxc://example.org/SEsfnsuifSDFSSEF"_s << QList{QUrl(u"www.example.org"_s)};
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

    QTest::newRow("mxc") << u"mxc://example.org/SEsfnsuifSDFSSEF"_s;
    QTest::newRow("matrixTo") << u"https://matrix.to/#/@alice:example.org"_s;
    QTest::newRow("noSpace") << u"testhttps://kde.org"_s;
}

void LinkPreviewerTest::linkPreviewsReject()
{
    QFETCH(QString, inputString);

    auto links = LinkPreviewer::linkPreviews(inputString);

    QCOMPARE(links.empty(), true);
}

QTEST_MAIN(LinkPreviewerTest)
#include "linkpreviewertest.moc"
