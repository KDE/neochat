// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <Quotient/connection.h>
#include <Quotient/quotient_common.h>
#include <Quotient/roommember.h>
#include <Quotient/syncdata.h>

#include "block.h"
#include "models/eventmessagecontentmodel.h"

#include "neochatconnection.h"
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
    connection = new NeoChatConnection;
}

void MessageContentModelTest::missingEvent()
{
    auto room = new TestUtils::TestRoom(connection, u"#firstRoom:kde.org"_s);
    auto model1 = EventMessageContentModel(room, u"$153456789:example.org"_s);

    QCOMPARE(model1.rowCount(), 1);
    QCOMPARE(model1.data(model1.index(0), MessageContentModel::ComponentTypeRole), Blocks::Loading);
    QCOMPARE(model1.data(model1.index(0), MessageContentModel::BlockRole).value<Blocks::TextBlock *>()->item()->plainText(), u"Loading…"_s);

    auto model2 = EventMessageContentModel(room, u"$153456789:example.org"_s, true);

    QCOMPARE(model2.rowCount(), 1);
    QCOMPARE(model2.data(model2.index(0), MessageContentModel::ComponentTypeRole), Blocks::Loading);
    QCOMPARE(model2.data(model1.index(0), MessageContentModel::BlockRole).value<Blocks::TextBlock *>()->item()->plainText(), u"Loading reply…"_s);

    room->syncNewEvents(u"test-min-sync.json"_s);
    QCOMPARE(model1.rowCount(), 2);
    QCOMPARE(model1.data(model1.index(0), MessageContentModel::ComponentTypeRole), Blocks::Author);
    QCOMPARE(model1.data(model1.index(1), MessageContentModel::ComponentTypeRole), Blocks::Text);
    QCOMPARE(model1.data(model1.index(1), MessageContentModel::BlockRole).value<Blocks::TextBlock *>()->item()->plainText(),
             u"<b>This is an example<br>text message</b>"_s);
}

QTEST_MAIN(MessageContentModelTest)
#include "messagecontentmodeltest.moc"
