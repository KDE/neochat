// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <Quotient/connection.h>
#include <Quotient/quotient_common.h>
#include <Quotient/roommember.h>
#include <Quotient/syncdata.h>

#include "models/messagecontentmodel.h"

#include "testutils.h"

using namespace Quotient;
using namespace Qt::Literals::StringLiterals;

class MessageContentModelTest : public QObject
{
    Q_OBJECT

private:
    Connection *connection = nullptr;

private Q_SLOTS:
    void initTestCase();

    void missingEvent();
};

void MessageContentModelTest::initTestCase()
{
    connection = Connection::makeMockConnection(QStringLiteral("@bob:kde.org"));
}

void MessageContentModelTest::missingEvent()
{
    auto room = new TestUtils::TestRoom(connection, QStringLiteral("#firstRoom:kde.org"));
    auto model1 = MessageContentModel(room, "$153456789:example.org"_L1);

    QCOMPARE(model1.rowCount(), 1);
    QCOMPARE(model1.data(model1.index(0), MessageContentModel::ComponentTypeRole), MessageComponentType::Loading);
    QCOMPARE(model1.data(model1.index(0), MessageContentModel::DisplayRole), "Loading"_L1);

    auto model2 = MessageContentModel(room, "$153456789:example.org"_L1, true);

    QCOMPARE(model2.rowCount(), 1);
    QCOMPARE(model2.data(model2.index(0), MessageContentModel::ComponentTypeRole), MessageComponentType::Loading);
    QCOMPARE(model2.data(model2.index(0), MessageContentModel::DisplayRole), "Loading reply"_L1);

    room->syncNewEvents(QLatin1String("test-min-sync.json"));
    QCOMPARE(model1.rowCount(), 2);
    QCOMPARE(model1.data(model1.index(0), MessageContentModel::ComponentTypeRole), MessageComponentType::Author);
    QCOMPARE(model1.data(model1.index(1), MessageContentModel::ComponentTypeRole), MessageComponentType::Text);
    QCOMPARE(model1.data(model1.index(1), MessageContentModel::DisplayRole), u"<b>This is an example<br>text message</b>"_s);
}

QTEST_MAIN(MessageContentModelTest)
#include "messagecontentmodeltest.moc"
