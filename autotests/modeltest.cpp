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
#include "models/messagecontentmodel.h"
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

// TODO: Add data to all models as relevant.

// Performs basic tests on all models in NeoChat
// When adding a new test, create the model first, then the tester, then initialize the model (e.g., setConnection and setRoom).
// That way, the models are also tested for whether they can handle having no connection etc.
class ModelTest : public QObject
{
    Q_OBJECT

private:
    NeoChatConnection *connection = nullptr;
    NeoChatRoom *room = nullptr;

    QString eventId;

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
    auto accountManager = new AccountManager(true, this);
    QSignalSpy spy(accountManager, &AccountManager::connectionAdded);
    connection = dynamic_cast<NeoChatConnection *>(accountManager->accounts()->front());
    const auto roomId = server.createRoom(u"@user:localhost:1234"_s);
    eventId = server.sendEvent(roomId,
                               u"m.room.message"_s,
                               QJsonObject{
                                   {u"body"_s, u"foo"_s},
                                   {u"msgtype"_s, u"m.text"_s},
                               });

    server.sendEvent(roomId,
                     u"m.room.message"_s,
                     QJsonObject{
                         {u"body"_s, u"asdf"_s},
                         {u"m.relates_to"_s,
                          QJsonObject{
                              {u"event_id"_s, u"$GEucSt3TfVl6DVpKEyeOlRsXzjLv2ZCVgSQuQclFg1o"_s},
                              {u"is_falling_back"_s, true},
                              {u"m.in_reply_to"_s, QJsonObject{{u"event_id"_s, u"$GEucSt3TfVl6DVpKEyeOlRsXzjLv2ZCVgSQuQclFg1o"_s}}},
                              {u"rel_type"_s, u"m.thread"_s},
                          }},
                         {u"msgtype"_s, u"m.text"_s},
                     });

    QSignalSpy syncSpy(connection, &Connection::syncDone);
    // We need to wait for two syncs, as the next one won't have the changes yet
    QVERIFY(syncSpy.wait());
    QVERIFY(syncSpy.wait());
    room = dynamic_cast<NeoChatRoom *>(connection->room(roomId));
    QVERIFY(room);
}

void ModelTest::testRoomTreeModel()
{
    auto roomTreeModel = new RoomTreeModel(this);
    auto tester = new QAbstractItemModelTester(roomTreeModel, roomTreeModel);
    tester->setUseFetchMore(true);
    roomTreeModel->setConnection(connection);
}

void ModelTest::testMessageContentModel()
{
    auto contentModel = std::make_unique<MessageContentModel>(room, eventId);
    auto tester = new QAbstractItemModelTester(contentModel.get());
    tester->setUseFetchMore(true);
}

void ModelTest::testEventMessageContentModel()
{
    auto model = std::make_unique<EventMessageContentModel>(room, eventId);
    auto tester = new QAbstractItemModelTester(model.get(), model.get());
    tester->setUseFetchMore(true);
}

void ModelTest::testThreadModel()
{
    auto model = new ThreadModel(eventId, room);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
}

void ModelTest::testThreadFetchModel()
{
    auto model = new ThreadFetchModel(new ThreadModel(eventId, room));
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
}

void ModelTest::testThreadChatBarModel()
{
    auto model = new ThreadChatBarModel(new ThreadModel(eventId, room), room);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
}

void ModelTest::testReactionModel()
{
    auto messageContentModel = std::make_unique<MessageContentModel>(room);
    auto model = new ReactionModel(messageContentModel.get(), eventId, room);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
}

void ModelTest::testPollAnswerModel()
{
    auto handler = std::make_unique<PollHandler>(room, eventId);
    auto model = new PollAnswerModel(handler.get());
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
}

void ModelTest::testLineModel()
{
    auto model = new LineModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    auto document = new QTextDocument(this);
    model->setDocument(document);
    document->setPlainText(u"foo\nbar\n\nbaz"_s);
}

void ModelTest::testSpaceChildrenModel()
{
    auto model = new SpaceChildrenModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setSpace(room);
}

void ModelTest::testItineraryModel()
{
    auto model = new ItineraryModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
}

void ModelTest::testPublicRoomListModel()
{
    auto model = new PublicRoomListModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setConnection(connection);
}

void ModelTest::testMessageFilterModel()
{
    auto timelineModel = new TimelineModel(this);
    auto model = new MessageFilterModel(this, timelineModel);
    auto tester = new QAbstractItemModelTester(model, model);
    timelineModel->setRoom(room);
    tester->setUseFetchMore(true);
}

void ModelTest::testThreePIdModel()
{
    auto model = new ThreePIdModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setConnection(connection);
}

void ModelTest::testMediaMessageFilterModel()
{
    auto timelineModel = new TimelineModel(this);
    auto messageFilterModel = new MessageFilterModel(this, timelineModel);
    auto model = new MediaMessageFilterModel(this, messageFilterModel);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    timelineModel->setRoom(room);
}

void ModelTest::testWebshortcutModel()
{
    auto model = new WebShortcutModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setSelectedText(u"Foo"_s);
}

void ModelTest::testTimelineMessageModel()
{
    auto model = new TimelineMessageModel();
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setRoom(room);
}

void ModelTest::testReadMarkerModel()
{
    auto model = std::make_unique<ReadMarkerModel>(eventId, room);
    auto tester = new QAbstractItemModelTester(model.get(), model.get());
    tester->setUseFetchMore(true);
}

void ModelTest::testSearchModel()
{
    auto model = new SearchModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setSearchText(u"foo"_s);
    model->setRoom(room);
}

void ModelTest::testStateModel()
{
    auto model = new StateModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setRoom(room);
}

void ModelTest::testTimelineModel()
{
    auto model = new TimelineModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setRoom(room);
}

void ModelTest::testStateKeysModel()
{
    auto model = new StateKeysModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setEventType(u"m.room.member"_s);
    model->setRoom(room);
}

void ModelTest::testPinnedMessageModel()
{
    auto model = new PinnedMessageModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setRoom(room);
}

void ModelTest::testUserListModel()
{
    auto model = new UserListModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setRoom(room);
}

void ModelTest::testStickerModel()
{
    auto model = new StickerModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setPackIndex(0);
    model->setRoom(room);
    auto imagePacksModel = new ImagePacksModel(this);
    model->setModel(imagePacksModel);
    imagePacksModel->setRoom(room);
    imagePacksModel->setShowEmoticons(true);
    imagePacksModel->setShowStickers(true);
}

void ModelTest::testPowerLevelModel()
{
    auto model = new PowerLevelModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
}

void ModelTest::testImagePacksModel()
{
    auto model = new ImagePacksModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setRoom(room);
    model->setShowEmoticons(true);
    model->setShowStickers(true);
}

void ModelTest::testCompletionModel()
{
    auto model = new CompletionModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setRoom(room);
    model->setAutoCompletionType(CompletionModel::Room);
    auto roomListModel = new RoomListModel(this);
    roomListModel->setConnection(connection);
    model->setRoomListModel(roomListModel);
}

void ModelTest::testRoomListModel()
{
    auto model = new RoomListModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setConnection(connection);
}

void ModelTest::testCommonRoomsModel()
{
    auto model = new CommonRoomsModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setConnection(connection);
    model->setUserId(u"@user:example.com"_s);
}

void ModelTest::testNotificationsModel()
{
    auto model = new NotificationsModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setConnection(connection);
}

void ModelTest::testLocationsModel()
{
    auto model = new LocationsModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setRoom(room);
}

void ModelTest::testServerListModel()
{
    auto model = new ServerListModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setConnection(connection);
}

void ModelTest::testEmojiModel()
{
    auto tester = new QAbstractItemModelTester(&EmojiModel::instance(), &EmojiModel::instance());
    tester->setUseFetchMore(true);
}

void ModelTest::testCustomEmojiModel()
{
    auto tester = new QAbstractItemModelTester(&CustomEmojiModel::instance(), &CustomEmojiModel::instance());
    tester->setUseFetchMore(true);
    CustomEmojiModel::instance().setConnection(connection);
}

void ModelTest::testPushRuleModel()
{
    auto model = new PushRuleModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setConnection(connection);
}

void ModelTest::testActionsModel()
{
    auto tester = new QAbstractItemModelTester(&ActionsModel::instance(), &ActionsModel::instance());
    tester->setUseFetchMore(true);
}

void ModelTest::testDevicesModel()
{
    auto model = new DevicesModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setConnection(connection);
}

void ModelTest::testUserDirectoryListModel()
{
    auto model = new UserDirectoryListModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setConnection(connection);
    model->setSearchText(u"foo"_s);
}

void ModelTest::testAccountEmoticonModel()
{
    auto model = new AccountEmoticonModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setConnection(connection);
}

void ModelTest::testPermissionsModel()
{
    auto model = new PermissionsModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setRoom(room);
}

void ModelTest::testLiveLocationsModel()
{
    auto model = new LiveLocationsModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setRoom(room);
}

void ModelTest::testRoomSortParameterModel()
{
    auto model = new RoomSortParameterModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
}

void ModelTest::testSortFilterRoomTreeModel()
{
    auto sourceModel = new RoomTreeModel(this);
    auto model = new SortFilterRoomTreeModel(sourceModel, sourceModel);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    sourceModel->setConnection(connection);
}

void ModelTest::testSortFilterSpaceListModel()
{
    auto sourceModel = new RoomListModel(this);
    auto model = new SortFilterSpaceListModel(sourceModel, sourceModel);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    sourceModel->setConnection(connection);
}

void ModelTest::testSortFilterRoomListModel()
{
    auto sourceModel = new RoomListModel(this);
    auto model = new SortFilterRoomListModel(sourceModel, sourceModel);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    sourceModel->setConnection(connection);
}

void ModelTest::testSpaceChildSortFilterModel()
{
    auto model = new SpaceChildSortFilterModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    auto spaceChildrenModel = new SpaceChildrenModel(this);
    model->setSourceModel(spaceChildrenModel);
    spaceChildrenModel->setSpace(nullptr);
}

void ModelTest::testStateFilterModel()
{
    auto model = new StateFilterModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    auto stateModel = new StateModel(this);
    model->setSourceModel(stateModel);
    stateModel->setRoom(room);
}

void ModelTest::testMessageContentFilterModel()
{
    auto model = new MessageContentFilterModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setSourceModel(ContentProvider::self().contentModelForEvent(room, eventId));
}

void ModelTest::testUserFilterModel()
{
    auto model = new UserFilterModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    auto userListModel = new UserListModel(this);
    model->setSourceModel(userListModel);
    userListModel->setRoom(room);
}

void ModelTest::testEmoticonFilterModel()
{
    auto model = new EmoticonFilterModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    auto accountEmoticonModel = new AccountEmoticonModel(this);
    model->setSourceModel(accountEmoticonModel);
    model->setShowEmojis(true);
    model->setShowStickers(true);
    accountEmoticonModel->setConnection(connection);
}

void ModelTest::testDevicesProxyModel()
{
    auto model = new DevicesProxyModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    auto devicesModel = new DevicesModel(this);
    model->setSourceModel(devicesModel);
    devicesModel->setConnection(dynamic_cast<NeoChatConnection *>(connection));
}

void ModelTest::testCompletionProxyModel()
{
    auto model = new CompletionProxyModel(this);
    auto tester = new QAbstractItemModelTester(model, model);
    tester->setUseFetchMore(true);
    model->setSourceModel(&EmojiModel::instance());
}

QTEST_MAIN(ModelTest)
#include "modeltest.moc"
