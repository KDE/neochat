// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <KLocalizedString>

#include <Quotient/connection.h>

#include "accountmanager.h"
#include "enums/neochatroomtype.h"
#include "models/roomtreemodel.h"
#include "models/sortfilterroomtreemodel.h"
#include "neochatroom.h"
#include "roommanager.h"
#include "server.h"

using namespace Quotient;

class TestSortFilterRoomTreeModel : public SortFilterRoomTreeModel
{
public:
    using SortFilterRoomTreeModel::lessThan;
    using SortFilterRoomTreeModel::SortFilterRoomTreeModel;
};

class RoomListOrderTest : public QObject
{
    Q_OBJECT

private:
    NeoChatConnection *connection = nullptr;
    Server server;
    QString normalRoomId;
    QString directChatId;
    QString favoriteRoomId;

    void waitForSync();
    QList<int> visibleCategories(const QSortFilterProxyModel *model) const;
    void verifyStrictOrdering(TestSortFilterRoomTreeModel *model);

private Q_SLOTS:
    void initTestCase();
    void testCategoryShownLater();
    void testAddDirectFirst();
};

void RoomListOrderTest::waitForSync()
{
    QSignalSpy syncSpy(connection, &Connection::syncDone);
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
}

QList<int> RoomListOrderTest::visibleCategories(const QSortFilterProxyModel *model) const
{
    QList<int> categories;
    for (int i = 0; i < model->rowCount(); i++) {
        categories += model->index(i, 0).data(RoomTreeModel::CategoryRole).toInt();
    }
    return categories;
}

void RoomListOrderTest::verifyStrictOrdering(TestSortFilterRoomTreeModel *model)
{
    const auto treeModel = model->sourceModel();
    for (int i = 0; i < treeModel->rowCount(); i++) {
        const auto left = treeModel->index(i, 0);
        QVERIFY2(!model->lessThan(left, left), qPrintable(u"category %1 is less than itself"_s.arg(i)));
        for (int j = 0; j < treeModel->rowCount(); j++) {
            const auto right = treeModel->index(j, 0);
            QVERIFY2(!(model->lessThan(left, right) && model->lessThan(right, left)),
                     qPrintable(u"categories %1 and %2 are both less than each other"_s.arg(i).arg(j)));
        }
    }
}

void RoomListOrderTest::initTestCase()
{
    Connection::setRoomType<NeoChatRoom>();
    server.start();
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("neochat"));
    auto accountManager = new AccountManager(true);
    QSignalSpy spy(accountManager, &AccountManager::connectionAdded);
    connection = dynamic_cast<NeoChatConnection *>(accountManager->accounts()->front());
    QVERIFY(connection);
    normalRoomId = server.createRoom(u"@user:localhost:1234"_s);
    directChatId = server.createDirectChat(u"@user:localhost:1234"_s, u"@other:localhost:1234"_s);
    favoriteRoomId = server.createRoom(u"@user:localhost:1234"_s, {u"m.favourite"_s});
    server.sendStateEvent(normalRoomId, u"m.room.name"_s, {}, {{u"name"_s, u"Shown"_s}});
    server.sendStateEvent(directChatId, u"m.room.name"_s, {}, {{u"name"_s, u"Shown too"_s}});
    server.sendStateEvent(favoriteRoomId, u"m.room.name"_s, {}, {{u"name"_s, u"Hidden"_s}});
    waitForSync();
    QVERIFY(connection->room(normalRoomId));
    QVERIFY(connection->room(directChatId));
    QVERIFY(connection->room(directChatId)->isDirectChat());
    QVERIFY(connection->room(favoriteRoomId));
    QVERIFY(connection->room(favoriteRoomId)->isFavourite());
    // Only set the connection now, since RoomTreeModel doesn't move rooms when they become direct chats.
    RoomManager::instance().setConnection(connection);
}

// BUG: 504380
void RoomListOrderTest::testCategoryShownLater()
{
    const auto treeModel = RoomManager::instance().roomTreeModel();
    QCOMPARE(treeModel->rowCount(treeModel->index(NeoChatRoomType::Favorite, 0)), 1);
    auto model = std::make_unique<TestSortFilterRoomTreeModel>(treeModel);
    const QList<int> expected{NeoChatRoomType::Favorite, NeoChatRoomType::Direct, NeoChatRoomType::Normal};
    QCOMPARE(visibleCategories(model.get()), expected);

    // Hide the favorite category; e.g. at startup it can be hidden until the space hierarchy is known.
    model->setFilterText(u"Shown"_s);
    QCOMPARE(visibleCategories(model.get()), (QList<int>{NeoChatRoomType::Direct, NeoChatRoomType::Normal}));

    // When it becomes visible again, it must be placed above the other categories without a full re-sort.
    model->setFilterText({});
    QCOMPARE(visibleCategories(model.get()), expected);
}

void RoomListOrderTest::testAddDirectFirst()
{
    const auto treeModel = RoomManager::instance().roomTreeModel();
    QCOMPARE(treeModel->rowCount(treeModel->index(NeoChatRoomType::Direct, 0)), 1);

    for (const auto mode : {SortFilterRoomTreeModel::Rooms, SortFilterRoomTreeModel::DirectChats, SortFilterRoomTreeModel::All}) {
        auto model = std::make_unique<TestSortFilterRoomTreeModel>(treeModel);
        model->setMode(mode);
        verifyStrictOrdering(model.get());
        if (mode == SortFilterRoomTreeModel::DirectChats) {
            QCOMPARE(visibleCategories(model.get()), (QList<int>{NeoChatRoomType::AddDirect, NeoChatRoomType::Direct}));
        }
    }
}

QTEST_GUILESS_MAIN(RoomListOrderTest)
#include "roomlistordertest.moc"
