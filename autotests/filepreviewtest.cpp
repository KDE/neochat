// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QProcess>
#include <QSignalSpy>
#include <QTest>

#include "config-neochat.h"
#include "enums/blocktype.h"
#include "filepreview.h"

using namespace Qt::StringLiterals;
using namespace Blocks;

class FilePreviewTest : public QObject
{
    Q_OBJECT

private:
    bool m_extractorAvailable = false;

private Q_SLOTS:
    void initTestCase();

    void fileTest_data();
    void fileTest();
};

void FilePreviewTest::initTestCase()
{
    // We need to figure out if the machine running the test has itinerary extractor.
    auto process = new QProcess(this);
    process->start("%1%2"_L1.arg(CMAKE_INSTALL_FULL_LIBEXECDIR_KF6, "/kitinerary-extractor"_L1), {"testString"_L1});
    process->waitForFinished();

    if (process->exitStatus() == QProcess::NormalExit) {
        m_extractorAvailable = true;
    }
}

void FilePreviewTest::fileTest_data()
{
    QTest::addColumn<QUrl>("path");
    QTest::addColumn<bool>("available");
    QTest::addColumn<Type>("typeExtractorAvailable");
    QTest::addColumn<Type>("typeExtractorUnavailable");

    QTest::newRow("remote") << QUrl("www.kde.org"_L1) << false << Other << Other;
    QTest::newRow("text") << QUrl::fromLocalFile("%1/TestText.txt"_L1.arg(DATA_DIR)) << true << Code << Code;
    QTest::newRow("itinerary") << QUrl::fromLocalFile("%1/eventreservation.ics"_L1.arg(DATA_DIR)) << true << Itinerary << Code;
    QTest::newRow("pdf") << QUrl::fromLocalFile("%1/KDE_Org.pdf"_L1.arg(DATA_DIR)) << true << Pdf << Pdf;
}

void FilePreviewTest::fileTest()
{
    QFETCH(QUrl, path);
    QFETCH(bool, available);
    QFETCH(Type, typeExtractorAvailable);
    QFETCH(Type, typeExtractorUnavailable);

    const auto loader = new FilePreviewBlockLoader(this, path);
    QSignalSpy spyAvailable(loader, &FilePreviewBlockLoader::blockAvailable);
    QSignalSpy spyUnavailable(loader, &FilePreviewBlockLoader::blockUnavailable);

    if (available) {
        QVERIFY(spyAvailable.wait(5000));
        QCOMPARE(spyAvailable.count(), 1);
        QCOMPARE(loader->state(), FilePreviewBlockLoader::Available);
        QCOMPARE(loader->previewBlock() != nullptr, true);
        QCOMPARE(loader->previewBlock()->type(), m_extractorAvailable ? typeExtractorAvailable : typeExtractorUnavailable);
    } else {
        if (loader->state() == FilePreviewBlockLoader::Loading) {
            QVERIFY(spyUnavailable.wait(5000));
            QCOMPARE(spyUnavailable.count(), 1);
        }
        QCOMPARE(loader->state(), FilePreviewBlockLoader::Unavailable);
        QCOMPARE(loader->previewBlock(), nullptr);
    }
}

QTEST_MAIN(FilePreviewTest)
#include "filepreviewtest.moc"
