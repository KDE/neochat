// SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QAbstractItemModelTester>
#include <QObject>
#include <QSignalSpy>
#include <QTest>
#include <QVariantList>

#include <Quotient/connection.h>

#include "accountmanager.h"
#include "contentprovider.h"
#include "enums/powerlevel.h"
#include "enums/roomsortparameter.h"
#include "models/accountemoticonmodel.h"
#include "models/actionsmodel.h"
#include "models/commonroomsmodel.h"
#include "models/completionmodel.h"
#include "models/completionproxymodel.h"
#include "models/customemojimodel.h"
#include "models/devicesmodel.h"
#include "models/devicesproxymodel.h"
#include "models/emojimodel.h"
#include "models/emoticonfiltermodel.h"
#include "models/eventmessagecontentmodel.h"
#include "models/imagepacksmodel.h"
#include "models/linemodel.h"
#include "models/livelocationsmodel.h"
#include "models/locationsmodel.h"
#include "models/messagecontentfiltermodel.h"
#include "models/notificationsmodel.h"
#include "models/permissionsmodel.h"
#include "models/pinnedmessagemodel.h"
#include "models/pollanswermodel.h"
#include "models/publicroomlistmodel.h"
#include "models/pushrulemodel.h"
#include "models/readmarkermodel.h"
#include "models/roomsortparametermodel.h"
#include "models/searchmodel.h"
#include "models/serverlistmodel.h"
#include "models/spacechildrenmodel.h"
#include "models/spacechildsortfiltermodel.h"
#include "models/statefiltermodel.h"
#include "models/statekeysmodel.h"
#include "models/statemodel.h"
#include "models/stickermodel.h"
#include "models/threadmodel.h"
#include "models/threepidmodel.h"
#include "models/userdirectorylistmodel.h"
#include "models/userfiltermodel.h"
#include "models/webshortcutmodel.h"
#include "neochatroom.h"
#include "pollhandler.h"
#include "roommanager.h"
#include "server.h"

using namespace Quotient;

// Performs basic tests on all models in NeoChat
// When adding a new test, create the model first, then the tester, then initialize the model (e.g., setConnection and setRoom).
// That way, the models are also tested for whether they can handle having no connection etc.
class ModelTest : public QObject
{
    Q_OBJECT

private:
    NeoChatConnection *connection = nullptr;
    NeoChatRoom *room = nullptr;

    Server server;

private Q_SLOTS:
    void initTestCase();
    void testRoomTreeModel();
    void testMessageContentModel();
    void testEventMessageContentModel();
    void testThreadModel();
    void testThreadFetchModel();
    void testThreadChatBarModel();
    void testReactionModel();
    void testPollAnswerModel();
    void testLineModel();
    void testSpaceChildrenModel();
    void testItineraryModel();
    void testPublicRoomListModel();
    void testMessageFilterModel();
    void testThreePIdModel();
    void testMediaMessageFilterModel();
    void testWebshortcutModel();
    void testTimelineMessageModel();
    void testReadMarkerModel();
    void testSearchModel();
    void testStateModel();
    void testTimelineModel();
    void testStateKeysModel();
    void testPinnedMessageModel();
    void testUserListModel();
    void testStickerModel();
    void testPowerLevelModel();
    void testImagePacksModel();
    void testCompletionModel();
    void testRoomListModel();
    void testCommonRoomsModel();
    void testNotificationsModel();
    void testLocationsModel();
    void testServerListModel();
    void testEmojiModel();
    void testCustomEmojiModel();
    void testPushRuleModel();
    void testActionsModel();
    void testDevicesModel();
    void testUserDirectoryListModel();
    void testAccountEmoticonModel();
    void testPermissionsModel();
    void testLiveLocationsModel();
    void testRoomSortParameterModel();
    void testSortFilterRoomTreeModel();
    void testSortFilterSpaceListModel();
    void testSortFilterRoomListModel();
    void testSpaceChildSortFilterModel();
    void testStateFilterModel();
    void testMessageContentFilterModel();
    void testUserFilterModel();
    void testEmoticonFilterModel();
    void testDevicesProxyModel();
    void testCompletionProxyModel();
};

void ModelTest::initTestCase()
{
    Connection::setRoomType<NeoChatRoom>();
    server.start();
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("neochat"));
    auto accountManager = new AccountManager(true);
    QSignalSpy spy(accountManager, &AccountManager::connectionAdded);
    connection = dynamic_cast<NeoChatConnection *>(accountManager->accounts()->front());
    auto roomId = server.createRoom(u"@user:localhost:1234"_s);
    // server.inviteUser(roomId, u"@invited:example.com"_s);
    // server.banUser(roomId, u"@banned:example.com"_s);
    // server.joinUser(roomId, u"@example:example.com"_s);

    QSignalSpy syncSpy(connection, &Connection::syncDone);
    // We need to wait for two syncs, as the next one won't have the changes yet
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
    room = dynamic_cast<NeoChatRoom *>(connection->room(roomId));
    QVERIFY(room);
}

void ModelTest::testRoomTreeModel()
{
    auto tester = new QAbstractItemModelTester(RoomManager::instance().roomTreeModel());
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testMessageContentModel()
{
    auto tester = new QAbstractItemModelTester(ContentProvider::self().contentModelForEvent(room, QString(/* TODO: use a proper event id here*/)));
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testEventMessageContentModel()
{
    auto model = new EventMessageContentModel(room, QString());
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testThreadModel()
{
    auto model = new ThreadModel(QString(), room);
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testThreadFetchModel()
{
    auto model = new ThreadFetchModel(new ThreadModel(QString(), room));
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testThreadChatBarModel()
{
    auto model = new ThreadChatBarModel(new ThreadModel(QString(), room), room);
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testReactionModel()
{
    auto model = new ReactionModel(new MessageContentModel(room), QString(), room);
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testPollAnswerModel()
{
    auto model = new PollAnswerModel(new PollHandler());
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testLineModel()
{
    auto model = new LineModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testSpaceChildrenModel()
{
    auto model = new SpaceChildrenModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testItineraryModel()
{
    auto model = new ItineraryModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testPublicRoomListModel()
{
    auto model = new PublicRoomListModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testMessageFilterModel()
{
    auto model = new MessageFilterModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testThreePIdModel()
{
    auto model = new ThreePIdModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testMediaMessageFilterModel()
{
    auto tester = new QAbstractItemModelTester(RoomManager::instance().mediaMessageFilterModel());
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testWebshortcutModel()
{
    auto model = new WebShortcutModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testTimelineMessageModel()
{
    auto model = new TimelineMessageModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testReadMarkerModel()
{
    auto model = new ReadMarkerModel(QString(), room);
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testSearchModel()
{
    auto model = new SearchModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testStateModel()
{
    auto model = new StateModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testTimelineModel()
{
    auto model = new TimelineModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testStateKeysModel()
{
    auto model = new StateKeysModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testPinnedMessageModel()
{
    auto model = new PinnedMessageModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testUserListModel()
{
    auto model = new UserListModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testStickerModel()
{
    auto model = new StickerModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testPowerLevelModel()
{
    auto model = new PowerLevelModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testImagePacksModel()
{
    auto model = new ImagePacksModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testCompletionModel()
{
    auto model = new CompletionModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testRoomListModel()
{
    auto model = new RoomListModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testCommonRoomsModel()
{
    auto model = new CommonRoomsModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testNotificationsModel()
{
    auto model = new NotificationsModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testLocationsModel()
{
    auto model = new LocationsModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testServerListModel()
{
    auto model = new ServerListModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testEmojiModel()
{
    auto tester = new QAbstractItemModelTester(&EmojiModel::instance());
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testCustomEmojiModel()
{
    auto tester = new QAbstractItemModelTester(&CustomEmojiModel::instance());
    tester->setUseFetchMore(true);
    // TODO
}

void ModelTest::testPushRuleModel()
{
    auto model = new PushRuleModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    model->setConnection(connection);
    // TODO: Add some push rules to the mock server
}

void ModelTest::testActionsModel()
{
    auto tester = new QAbstractItemModelTester(&ActionsModel::instance());
    tester->setUseFetchMore(true);
}

void ModelTest::testDevicesModel()
{
    // TODO: Make the server return some dummy devices
    auto model = new DevicesModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    model->setConnection(connection);
}

void ModelTest::testUserDirectoryListModel()
{
    auto model = new UserDirectoryListModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    model->setConnection(connection);
    model->setSearchText(u"foo"_s);
    // TODO implement user directory model in mock server
}

void ModelTest::testAccountEmoticonModel()
{
    auto model = new AccountEmoticonModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
}

void ModelTest::testPermissionsModel()
{
    auto model = new PermissionsModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    model->setRoom(room);
}

void ModelTest::testLiveLocationsModel()
{
    auto model = new LiveLocationsModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO set room
}

void ModelTest::testRoomSortParameterModel()
{
    auto model = new RoomSortParameterModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
}

void ModelTest::testSortFilterRoomTreeModel()
{
    auto tester = new QAbstractItemModelTester(RoomManager::instance().sortFilterRoomTreeModel());
    tester->setUseFetchMore(true);
}

void ModelTest::testSortFilterSpaceListModel()
{
    // TODO make sure we have some spaces
    auto tester = new QAbstractItemModelTester(RoomManager::instance().sortFilterSpaceListModel());
    tester->setUseFetchMore(true);
}

void ModelTest::testSortFilterRoomListModel()
{
    // TODO make sure we have some rooms
    auto tester = new QAbstractItemModelTester(RoomManager::instance().sortFilterRoomListModel());
    tester->setUseFetchMore(true);
}

void ModelTest::testSpaceChildSortFilterModel()
{
    auto model = new SpaceChildSortFilterModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    auto spaceChildrenModel = new SpaceChildrenModel();
    model->setSourceModel(spaceChildrenModel);
    spaceChildrenModel->setSpace(nullptr); // TODO create a real space
}

void ModelTest::testStateFilterModel()
{
    auto model = new StateFilterModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    auto stateModel = new StateModel();
    model->setSourceModel(stateModel);
    stateModel->setRoom(room);
    // TODO the dummy room probably already has some state?
}

void ModelTest::testMessageContentFilterModel()
{
    auto model = new MessageContentFilterModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    model->setSourceModel(ContentProvider::self().contentModelForEvent(room, QString(/* TODO: use a proper event id here*/)));
}

void ModelTest::testUserFilterModel()
{
    auto model = new UserFilterModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    model->setSourceModel(RoomManager::instance().userListModel());
    // TODO open room with some users
}

void ModelTest::testEmoticonFilterModel()
{
    auto model = new EmoticonFilterModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    // TODO: Add some emoticons to the account; requires extending the server first
    auto accountEmoticonModel = new AccountEmoticonModel();
    model->setSourceModel(accountEmoticonModel);
    model->setShowEmojis(true);
    model->setShowStickers(true);
}

void ModelTest::testDevicesProxyModel()
{
    auto model = new DevicesProxyModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    auto devicesModel = new DevicesModel();
    model->setSourceModel(devicesModel);
    devicesModel->setConnection(dynamic_cast<NeoChatConnection *>(connection));
}

void ModelTest::testCompletionProxyModel()
{
    auto model = new CompletionProxyModel();
    auto tester = new QAbstractItemModelTester(model);
    tester->setUseFetchMore(true);
    model->setSourceModel(&EmojiModel::instance());
}

QTEST_MAIN(ModelTest)
#include "modeltest.moc"
