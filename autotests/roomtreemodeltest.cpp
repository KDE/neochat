// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include <QAbstractItemModelTester>
#include <QTest>

#include "enums/neochatroomtype.h"
#include "models/roomtreemodel.h"
#include "neochatconnection.h"
#include "testutils.h"

using namespace Quotient;

class RoomTreeModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testTreeModel();
};

void RoomTreeModelTest::testTreeModel()
{
    auto connection = new NeoChatConnection;
    Connection::makeMockConnection(connection, QStringLiteral("@bob:kde.org"));

    auto room = dynamic_cast<NeoChatRoom *>(new TestUtils::TestRoom(connection, QStringLiteral("#myroom:kde.org"), QStringLiteral("test-min-sync.json")));
    QVERIFY(room);
    connection->addRoom(room);

    RoomTreeModel model;
    model.setConnection(connection);
    QAbstractItemModelTester tester(&model);

    QCOMPARE(model.rowCount(), static_cast<int>(NeoChatRoomType::TypesCount));

    // Check data category
    auto category = static_cast<int>(NeoChatRoomType::typeForRoom(room));
    auto normalCategoryIdx = model.index(category, 0);
    QCOMPARE(model.data(normalCategoryIdx, RoomTreeModel::DisplayNameRole).toString(), QStringLiteral("Normal"));
    QCOMPARE(model.data(normalCategoryIdx, RoomTreeModel::DelegateTypeRole).toString(), QStringLiteral("section"));
    QCOMPARE(model.data(normalCategoryIdx, RoomTreeModel::IconRole).toString(), QStringLiteral("group"));
    QCOMPARE(model.data(normalCategoryIdx, RoomTreeModel::CategoryRole).toInt(), category);
    QCOMPARE(model.rowCount(normalCategoryIdx), 1);

    // Check data room
    auto roomIdx = model.index(0, 0, normalCategoryIdx);
    QCOMPARE(model.data(roomIdx, RoomTreeModel::CurrentRoomRole).value<NeoChatRoom *>(), room);
    QCOMPARE(model.data(roomIdx, RoomTreeModel::CategoryRole).toInt(), category);

    QVERIFY(false);
}

QTEST_MAIN(RoomTreeModelTest)

#include "roomtreemodeltest.moc"
