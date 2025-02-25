// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "neochatroom.h"

#include <QFileInfo>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QMimeDatabase>
#include <QTemporaryFile>

#include <Quotient/events/eventcontent.h>
#include <Quotient/events/eventrelation.h>
#include <Quotient/events/roommessageevent.h>
#include <Quotient/jobs/basejob.h>
#include <Quotient/quotient_common.h>
#include <qcoro/qcorosignal.h>

#include <Quotient/avatar.h>
#include <Quotient/connection.h>
#include <Quotient/csapi/account-data.h>
#include <Quotient/csapi/directory.h>
#include <Quotient/csapi/pushrules.h>
#include <Quotient/csapi/redaction.h>
#include <Quotient/csapi/report_content.h>
#include <Quotient/csapi/room_state.h>
#include <Quotient/csapi/rooms.h>
#include <Quotient/csapi/typing.h>
#include <Quotient/events/encryptionevent.h>
#include <Quotient/events/reactionevent.h>
#include <Quotient/events/redactionevent.h>
#include <Quotient/events/roomavatarevent.h>
#include <Quotient/events/roomcanonicalaliasevent.h>
#include <Quotient/events/roommemberevent.h>
#include <Quotient/events/roompowerlevelsevent.h>
#include <Quotient/events/simplestateevents.h>
#include <Quotient/jobs/downloadfilejob.h>
#include <Quotient/qt_connection_util.h>

#include "chatbarcache.h"
#include "clipboard.h"
#include "eventhandler.h"
#include "events/pollevent.h"
#include "filetransferpseudojob.h"
#include "neochatconfig.h"
#include "neochatconnection.h"
#include "neochatroommember.h"
#include "roomlastmessageprovider.h"
#include "spacehierarchycache.h"
#include "texthandler.h"
#include "urlhelper.h"

#ifndef Q_OS_ANDROID
#include <KIO/Job>
#include <KIO/JobTracker>
#endif
#include <KJobTrackerInterface>
#include <KLocalizedString>

using namespace Quotient;

NeoChatRoom::NeoChatRoom(Connection *connection, QString roomId, JoinState joinState)
    : Room(connection, std::move(roomId), joinState)
{
    m_mainCache = new ChatBarCache(this);
    m_editCache = new ChatBarCache(this);
    m_threadCache = new ChatBarCache(this);

    connect(connection, &Connection::accountDataChanged, this, &NeoChatRoom::updatePushNotificationState);
    connect(this, &Room::fileTransferCompleted, this, [this] {
        setFileUploadingProgress(0);
        setHasFileUploading(false);
    });
    connect(this, &Room::fileTransferCompleted, this, [this](QString eventId) {
        const auto evtIt = findInTimeline(eventId);
        if (evtIt != messageEvents().rend()) {
            const auto m_event = evtIt->viewAs<RoomEvent>();
            QString mxcUrl;
            if (auto event = eventCast<const Quotient::RoomMessageEvent>(m_event)) {
                if (event->has<EventContent::FileContentBase>()) {
                    mxcUrl = event->get<EventContent::FileContentBase>()->url().toString();
                }
            } else if (auto event = eventCast<const Quotient::StickerEvent>(m_event)) {
                mxcUrl = event->image().url().toString();
            }
            if (mxcUrl.isEmpty()) {
                return;
            }
            auto localPath = this->fileTransferInfo(eventId).localPath.toLocalFile();
            auto config = KSharedConfig::openStateConfig(u"neochatdownloads"_s)->group(u"downloads"_s);
            config.writePathEntry(mxcUrl.mid(6), localPath);
        }
    });

    connect(this, &Room::addedMessages, this, &NeoChatRoom::readMarkerLoadedChanged);
    connect(this, &Room::aboutToAddHistoricalMessages, this, &NeoChatRoom::cleanupExtraEventRange);
    connect(this, &Room::aboutToAddNewMessages, this, &NeoChatRoom::cleanupExtraEventRange);

    const auto &roomLastMessageProvider = RoomLastMessageProvider::self();

    if (roomLastMessageProvider.hasKey(id())) {
        auto eventJson = QJsonDocument::fromJson(roomLastMessageProvider.read(id())).object();
        if (!eventJson.isEmpty()) {
            auto event = loadEvent<RoomEvent>(eventJson);

            if (event != nullptr) {
                m_cachedEvent = std::move(event);
            }
        }
    }
    connect(this, &Room::addedMessages, this, &NeoChatRoom::cacheLastEvent);

    connect(this, &Quotient::Room::eventsHistoryJobChanged, this, &NeoChatRoom::lastActiveTimeChanged);

    connect(this, &Room::joinStateChanged, this, [this](JoinState oldState, JoinState newState) {
        if (oldState == JoinState::Invite && newState != JoinState::Invite) {
            Q_EMIT isInviteChanged();
        }
    });
    connect(this, &Room::displaynameChanged, this, &NeoChatRoom::displayNameChanged);

    connect(
        this,
        &Room::baseStateLoaded,
        this,
        [this]() {
            updatePushNotificationState(u"m.push_rules"_s);

            Q_EMIT canEncryptRoomChanged();
        },
        Qt::SingleShotConnection);
    connect(this, &Room::changed, this, [this] {
        Q_EMIT canEncryptRoomChanged();
        Q_EMIT parentIdsChanged();
        Q_EMIT canonicalParentChanged();
        Q_EMIT readOnlyChanged();
    });
    connect(connection, &Connection::capabilitiesLoaded, this, &NeoChatRoom::maxRoomVersionChanged);
    connect(this, &Room::changed, this, [this]() {
        Q_EMIT defaultUrlPreviewStateChanged();
    });
    connect(this, &Room::accountDataChanged, this, [this](QString type) {
        if (type == "org.matrix.room.preview_urls"_L1) {
            Q_EMIT urlPreviewEnabledChanged();
        }
    });
    connect(&SpaceHierarchyCache::instance(), &SpaceHierarchyCache::spaceHierarchyChanged, this, [this]() {
        if (isSpace()) {
            Q_EMIT childrenNotificationCountChanged();
            Q_EMIT childrenHaveHighlightNotificationsChanged();
        }
    });
    connect(&SpaceHierarchyCache::instance(), &SpaceHierarchyCache::spaceNotifcationCountChanged, this, [this](const QStringList &spaces) {
        if (spaces.contains(id())) {
            Q_EMIT childrenNotificationCountChanged();
            Q_EMIT childrenHaveHighlightNotificationsChanged();
        }
    });

    const auto neochatconnection = static_cast<NeoChatConnection *>(connection);
    Q_ASSERT(neochatconnection);
    connect(neochatconnection, &NeoChatConnection::globalUrlPreviewEnabledChanged, this, &NeoChatRoom::urlPreviewEnabledChanged);
}

bool NeoChatRoom::visible() const
{
    return m_visible;
}

void NeoChatRoom::setVisible(bool visible)
{
    m_visible = visible;

    if (!visible) {
        m_memberObjects.clear();
        m_eventContentModels.clear();
        m_threadModels.clear();
    }
}

int NeoChatRoom::contextAwareNotificationCount() const
{
    // DOn't include spaces, rooms that the user hasn't joined and rooms where the user has joined the successor.
    if (isSpace() || joinState() != JoinState::Join || successor(JoinState::Join) != nullptr) {
        return 0;
    }
    if (m_currentPushNotificationState == PushNotificationState::Mute) {
        return 0;
    }
    if (m_currentPushNotificationState == PushNotificationState::MentionKeyword || isLowPriority()) {
        return int(highlightCount());
    }
    return int(notificationCount());
}

bool NeoChatRoom::hasFileUploading() const
{
    return m_hasFileUploading;
}

void NeoChatRoom::setHasFileUploading(bool value)
{
    if (value == m_hasFileUploading) {
        return;
    }
    m_hasFileUploading = value;
    Q_EMIT hasFileUploadingChanged();
}

int NeoChatRoom::fileUploadingProgress() const
{
    return m_fileUploadingProgress;
}

void NeoChatRoom::setFileUploadingProgress(int value)
{
    if (m_fileUploadingProgress == value) {
        return;
    }
    m_fileUploadingProgress = value;
    Q_EMIT fileUploadingProgressChanged();
}

void NeoChatRoom::uploadFile(const QUrl &url, const QString &body)
{
    doUploadFile(url, body);
}

QCoro::Task<void> NeoChatRoom::doUploadFile(QUrl url, QString body)
{
    if (url.isEmpty()) {
        co_return;
    }

    auto mime = QMimeDatabase().mimeTypeForUrl(url);
    url.setScheme("file"_L1);
    QFileInfo fileInfo(url.isLocalFile() ? url.toLocalFile() : url.toString());
    EventContent::FileContentBase *content;
    if (mime.name().startsWith("image/"_L1)) {
        QImage image(url.toLocalFile());
        content = new EventContent::ImageContent(url, fileInfo.size(), mime, image.size(), fileInfo.fileName());
    } else if (mime.name().startsWith("audio/"_L1)) {
        content = new EventContent::AudioContent(url, fileInfo.size(), mime, fileInfo.fileName());
    } else if (mime.name().startsWith("video/"_L1)) {
        QMediaPlayer player;
        player.setSource(url);
        co_await qCoro(&player, &QMediaPlayer::mediaStatusChanged);
        auto resolution = player.metaData().value(QMediaMetaData::Resolution).toSize();
        content = new EventContent::VideoContent(url, fileInfo.size(), mime, resolution, fileInfo.fileName());
    } else {
        content = new EventContent::FileContent(url, fileInfo.size(), mime, fileInfo.fileName());
    }
    QString txnId = postFile(body.isEmpty() ? url.fileName() : body, std::unique_ptr<EventContent::FileContentBase>(content));
    setHasFileUploading(true);
    connect(this, &Room::fileTransferCompleted, [this, txnId](const QString &id, FileSourceInfo) {
        if (id == txnId) {
            setFileUploadingProgress(0);
            setHasFileUploading(false);
        }
    });
    connect(this, &Room::fileTransferFailed, [this, txnId](const QString &id, const QString & /*error*/) {
        if (id == txnId) {
            setFileUploadingProgress(0);
            setHasFileUploading(false);
        }
    });
    connect(this, &Room::fileTransferProgress, [this, txnId](const QString &id, qint64 progress, qint64 total) {
        if (id == txnId) {
            setFileUploadingProgress(int(float(progress) / float(total) * 100));
        }
    });
#ifndef Q_OS_ANDROID
    auto job = new FileTransferPseudoJob(FileTransferPseudoJob::Upload, url.toLocalFile(), txnId);
    connect(this, &Room::fileTransferProgress, job, &FileTransferPseudoJob::fileTransferProgress);
    connect(this, &Room::fileTransferCompleted, job, &FileTransferPseudoJob::fileTransferCompleted);
    connect(this, &Room::fileTransferFailed, job, [this, job, txnId] {
        auto info = fileTransferInfo(txnId);
        if (info.status == FileTransferInfo::Cancelled) {
            job->fileTransferCanceled(txnId);
        } else {
            job->fileTransferFailed(txnId);
        }
    });
    connect(job, &FileTransferPseudoJob::cancelRequested, this, &Room::cancelFileTransfer);
    KIO::getJobTracker()->registerJob(job);
    job->start();
#endif
}

void NeoChatRoom::acceptInvitation()
{
    connection()->joinRoom(id());
}

void NeoChatRoom::forget()
{
    QStringList roomIds{id()};

    NeoChatRoom *predecessor = this;
    while (predecessor = dynamic_cast<NeoChatRoom *>(predecessor->predecessor(JoinState::Join)), predecessor && !roomIds.contains(predecessor->id())) {
        roomIds += predecessor->id();
    }

    for (const auto &id : roomIds) {
        connection()->forgetRoom(id);
    }
}

void NeoChatRoom::sendTypingNotification(bool isTyping)
{
    connection()->callApi<SetTypingJob>(BackgroundRequest, localMember().id(), id(), isTyping, 10000);
}

const RoomEvent *NeoChatRoom::lastEvent() const
{
    for (auto timelineItem = messageEvents().rbegin(); timelineItem < messageEvents().rend(); timelineItem++) {
        const RoomEvent *event = timelineItem->get();

        if (is<RedactionEvent>(*event) || is<ReactionEvent>(*event)) {
            continue;
        }
        if (event->isRedacted()) {
            continue;
        }

        if (event->isStateEvent() && !NeoChatConfig::showStateEvent()) {
            continue;
        }

        if (auto roomMemberEvent = eventCast<const RoomMemberEvent>(event)) {
            if ((roomMemberEvent->isJoin() || roomMemberEvent->isLeave()) && !NeoChatConfig::showLeaveJoinEvent()) {
                continue;
            } else if (roomMemberEvent->isRename() && !roomMemberEvent->isJoin() && !roomMemberEvent->isLeave() && !NeoChatConfig::showRename()) {
                continue;
            } else if (roomMemberEvent->isAvatarUpdate() && !roomMemberEvent->isJoin() && !roomMemberEvent->isLeave() && !NeoChatConfig::showAvatarUpdate()) {
                continue;
            }
        }
        if (event->isStateEvent() && static_cast<const StateEvent &>(*event).repeatsState()) {
            continue;
        }

        if (auto roomEvent = eventCast<const RoomMessageEvent>(event)) {
            if (!roomEvent->replacedEvent().isEmpty() && roomEvent->replacedEvent() != roomEvent->id()) {
                continue;
            }
        }

        if (connection()->isIgnored(event->senderId())) {
            continue;
        }

        if (auto lastEvent = eventCast<const StateEvent>(event)) {
            return lastEvent;
        }

        if (auto lastEvent = eventCast<const RoomMessageEvent>(event)) {
            return lastEvent;
        }
        if (auto lastEvent = eventCast<const PollStartEvent>(event)) {
            return lastEvent;
        }
    }

    if (m_cachedEvent != nullptr) {
        return std::to_address(m_cachedEvent);
    }

    return nullptr;
}

void NeoChatRoom::cacheLastEvent()
{
    auto event = lastEvent();
    if (event != nullptr) {
        auto &roomLastMessageProvider = RoomLastMessageProvider::self();

        auto eventJson = QJsonDocument(event->fullJson()).toJson(QJsonDocument::Compact);
        roomLastMessageProvider.write(id(), eventJson);

        auto uniqueEvent = loadEvent<RoomEvent>(event->fullJson());

        if (event != nullptr) {
            m_cachedEvent = std::move(uniqueEvent);
        }
    }
}

bool NeoChatRoom::lastEventIsSpoiler() const
{
    if (auto event = lastEvent()) {
        if (auto e = eventCast<const RoomMessageEvent>(event)) {
            if (e->has<EventContent::TextContent>() && e->content() && e->mimeType().name() == "text/html"_L1) {
                auto htmlBody = e->get<EventContent::TextContent>()->body;
                return htmlBody.contains("data-mx-spoiler"_L1);
            }
        }
    }
    return false;
}

bool NeoChatRoom::isEventHighlighted(const RoomEvent *e) const
{
    return highlights.contains(e);
}

void NeoChatRoom::checkForHighlights(const Quotient::TimelineItem &ti)
{
    auto localMember = this->localMember();
    if (ti->senderId() == localMember.id()) {
        return;
    }
    if (auto *e = ti.viewAs<RoomMessageEvent>()) {
        const auto &text = e->plainBody();
        if (text.contains(localMember.id()) || text.contains(localMember.disambiguatedName())) {
            highlights.insert(e);
        }
    }
}

void NeoChatRoom::onAddNewTimelineEvents(timeline_iter_t from)
{
    std::for_each(from, messageEvents().cend(), [this](const TimelineItem &ti) {
        checkForHighlights(ti);
    });
}

void NeoChatRoom::onAddHistoricalTimelineEvents(rev_iter_t from)
{
    std::for_each(from, messageEvents().crend(), [this](const TimelineItem &ti) {
        checkForHighlights(ti);
    });
}

void NeoChatRoom::onRedaction(const RoomEvent &prevEvent, const RoomEvent & /*after*/)
{
    if (const auto &e = eventCast<const ReactionEvent>(&prevEvent)) {
        if (auto relatedEventId = e->eventId(); !relatedEventId.isEmpty()) {
            Q_EMIT updatedEvent(relatedEventId);
        }
    }
}

QDateTime NeoChatRoom::lastActiveTime()
{
    if (timelineSize() == 0) {
        if (m_cachedEvent != nullptr) {
            return m_cachedEvent->originTimestamp();
        }
        return QDateTime();
    }

    if (auto event = lastEvent()) {
        return event->originTimestamp();
    }

    // no message found, take last event
    return messageEvents().rbegin()->get()->originTimestamp();
}

QUrl NeoChatRoom::avatarMediaUrl() const
{
    if (const auto avatar = Room::avatarUrl(); !avatar.isEmpty()) {
        return avatar;
    }

    // Use the first (excluding self) user's avatar for direct chats
    const auto directChatMembers = this->directChatMembers();
    for (const auto member : directChatMembers) {
        if (member != localMember()) {
            return member.avatarUrl();
        }
    }

    return {};
}

void NeoChatRoom::changeAvatar(const QUrl &localFile)
{
    const auto job = connection()->uploadFile(localFile.toLocalFile());
    if (isJobPending(job)) {
        connect(job, &BaseJob::success, this, [this, job] {
            connection()->callApi<SetRoomStateWithKeyJob>(id(), "m.room.avatar"_L1, QString(), QJsonObject{{"url"_L1, job->contentUri().toString()}});
        });
    }
}

QString msgTypeToString(MessageEventType msgType)
{
    switch (msgType) {
    case MessageEventType::Text:
        return "m.text"_L1;
    case MessageEventType::File:
        return "m.file"_L1;
    case MessageEventType::Audio:
        return "m.audio"_L1;
    case MessageEventType::Emote:
        return "m.emote"_L1;
    case MessageEventType::Image:
        return "m.image"_L1;
    case MessageEventType::Video:
        return "m.video"_L1;
    case MessageEventType::Notice:
        return "m.notice"_L1;
    case MessageEventType::Location:
        return "m.location"_L1;
    default:
        return "m.text"_L1;
    }
}

void NeoChatRoom::toggleReaction(const QString &eventId, const QString &reaction)
{
    if (eventId.isEmpty() || reaction.isEmpty()) {
        return;
    }

    const auto eventIt = findInTimeline(eventId);
    if (eventIt == historyEdge()) {
        return;
    }

    const auto &evt = **eventIt;

    QStringList redactEventIds; // What if there are multiple reaction events?

    const auto &annotations = relatedEvents(evt, EventRelation::AnnotationType);
    if (!annotations.isEmpty()) {
        for (const auto &a : annotations) {
            if (auto e = eventCast<const ReactionEvent>(a)) {
                if (e->key() != reaction) {
                    continue;
                }

                if (e->senderId() == localMember().id()) {
                    redactEventIds.push_back(e->id());
                    break;
                }
            }
        }
    }

    if (!redactEventIds.isEmpty()) {
        for (const auto &redactEventId : redactEventIds) {
            redactEvent(redactEventId);
        }
    } else {
        postReaction(eventId, reaction);
    }
}

bool NeoChatRoom::containsUser(const QString &userID) const
{
    return memberState(userID) != Membership::Leave;
}

bool NeoChatRoom::canSendEvent(const QString &eventType) const
{
    auto plEvent = currentState().get<RoomPowerLevelsEvent>();
    if (!plEvent) {
        return false;
    }
    auto pl = plEvent->powerLevelForEvent(eventType);
    auto currentPl = plEvent->powerLevelForUser(localMember().id());

    return currentPl >= pl;
}

bool NeoChatRoom::canSendState(const QString &eventType) const
{
    auto plEvent = currentState().get<RoomPowerLevelsEvent>();
    if (!plEvent) {
        return false;
    }
    auto pl = plEvent->powerLevelForState(eventType);
    auto currentPl = plEvent->powerLevelForUser(localMember().id());

    return currentPl >= pl;
}

bool NeoChatRoom::readMarkerLoaded() const
{
    const auto it = findInTimeline(lastFullyReadEventId());
    return it != historyEdge();
}

bool NeoChatRoom::isInvite() const
{
    return joinState() == JoinState::Invite;
}

bool NeoChatRoom::readOnly() const
{
    return !canSendEvent("m.room.message"_L1);
}

bool NeoChatRoom::isUserBanned(const QString &user) const
{
    auto roomMemberEvent = currentState().get<RoomMemberEvent>(user);
    if (!roomMemberEvent) {
        return false;
    }
    return roomMemberEvent->membership() == Membership::Ban;
}

void NeoChatRoom::deleteMessagesByUser(const QString &user, const QString &reason)
{
    doDeleteMessagesByUser(user, reason);
}

QString NeoChatRoom::historyVisibility() const
{
    if (auto stateEvent = currentState().get("m.room.history_visibility"_L1)) {
        return stateEvent->contentJson()["history_visibility"_L1].toString();
    }
    return {};
}

void NeoChatRoom::setHistoryVisibility(const QString &historyVisibilityRule)
{
    if (!canSendState("m.room.history_visibility"_L1)) {
        qWarning() << "Power level too low to set history visibility";
        return;
    }

    setState("m.room.history_visibility"_L1, {}, QJsonObject{{"history_visibility"_L1, historyVisibilityRule}});
    // Not emitting historyVisibilityChanged() here, since that would override the change in the UI with the *current* value, which is not the *new* value.
}

bool NeoChatRoom::defaultUrlPreviewState() const
{
    auto urlPreviewsDisabled = currentState().get("org.matrix.room.preview_urls"_L1);

    // Some rooms will not have this state event set so check for a nullptr return.
    if (urlPreviewsDisabled != nullptr) {
        return !urlPreviewsDisabled->contentJson()["disable"_L1].toBool();
    } else {
        return false;
    }
}

void NeoChatRoom::setDefaultUrlPreviewState(const bool &defaultUrlPreviewState)
{
    if (!canSendState("org.matrix.room.preview_urls"_L1)) {
        qWarning() << "Power level too low to set the default URL preview state for the room";
        return;
    }

    /**
     * Note the org.matrix.room.preview_urls room state event is completely undocumented
     * so here it is because I'm nice.
     *
     * Also note this is a different event to org.matrix.room.preview_urls for room
     * account data, because even though it has the same name and content it's totally different.
     *
     * {
     *  "content": {
     *      "disable": false
     *  },
     *  "origin_server_ts": 1673115224071,
     *  "sender": "@bob:kde.org",
     *  "state_key": "",
     *  "type": "org.matrix.room.preview_urls",
     *  "unsigned": {
     *      "replaces_state": "replaced_event_id",
     *      "prev_content": {
     *          "disable": true
     *      },
     *      "prev_sender": "@jeff:kde.org",
     *      "age": 99
     *  },
     *  "event_id": "$event_id",
     *  "room_id": "!room_id:kde.org"
     * }
     *
     * You just have to set disable to true to disable URL previews by default.
     */
    setState("org.matrix.room.preview_urls"_L1, {}, QJsonObject{{"disable"_L1, !defaultUrlPreviewState}});
}

bool NeoChatRoom::urlPreviewEnabled() const
{
    if (!static_cast<NeoChatConnection *>(connection())->globalUrlPreviewEnabled()) {
        return false;
    }
    if (hasAccountData("org.matrix.room.preview_urls"_L1)) {
        return !accountData("org.matrix.room.preview_urls"_L1)->contentJson()["disable"_L1].toBool();
    } else {
        return defaultUrlPreviewState();
    }
}

void NeoChatRoom::setUrlPreviewEnabled(const bool &urlPreviewEnabled)
{
    /**
     * Once again this is undocumented and even though the name and content are the
     * same this is a different event to the org.matrix.room.preview_urls room state event.
     *
     * {
     *  "content": {
     *      "disable": true
     *  }
     *  "type": "org.matrix.room.preview_urls",
     * }
     */
    connection()->callApi<SetAccountDataPerRoomJob>(localMember().id(),
                                                    id(),
                                                    "org.matrix.room.preview_urls"_L1,
                                                    QJsonObject{{"disable"_L1, !urlPreviewEnabled}});
}

void NeoChatRoom::setUserPowerLevel(const QString &userID, const int &powerLevel)
{
    if (joinedCount() <= 1) {
        qWarning() << "Cannot modify the power level of the only user";
        return;
    }
    if (!canSendState("m.room.power_levels"_L1)) {
        qWarning() << "Power level too low to set user power levels";
        return;
    }
    if (!isMember(userID)) {
        qWarning() << "User is not a member of this room so power level cannot be set";
        return;
    }
    int clampPowerLevel = std::clamp(powerLevel, -1, 100);

    auto powerLevelContent = currentState().get("m.room.power_levels"_L1)->contentJson();
    auto powerLevelUserOverrides = powerLevelContent["users"_L1].toObject();

    if (powerLevelUserOverrides[userID] != clampPowerLevel) {
        powerLevelUserOverrides[userID] = clampPowerLevel;
        powerLevelContent["users"_L1] = powerLevelUserOverrides;

        setState("m.room.power_levels"_L1, {}, powerLevelContent);
    }
}

QCoro::Task<void> NeoChatRoom::doDeleteMessagesByUser(const QString &user, QString reason)
{
    QStringList events;
    for (const auto &event : messageEvents()) {
        if (event->senderId() == user && !event->isRedacted() && !event.viewAs<RedactionEvent>() && !event->isStateEvent()) {
            events += event->id();
        }
    }
    for (const auto &e : events) {
        auto job = connection()->callApi<RedactEventJob>(id(), QString::fromLatin1(QUrl::toPercentEncoding(e)), connection()->generateTxnId(), reason);
        co_await qCoro(job.get(), &BaseJob::finished);
        if (job->error() != BaseJob::Success) {
            qWarning() << "Error: \"" << job->error() << "\" while deleting messages. Aborting";
            break;
        }
    }
}

bool NeoChatRoom::hasParent() const
{
    return currentState().eventsOfType("m.space.parent"_L1).size() > 0;
}

QList<QString> NeoChatRoom::parentIds() const
{
    auto parentEvents = currentState().eventsOfType("m.space.parent"_L1);
    QList<QString> parentIds;
    for (const auto &parentEvent : parentEvents) {
        if (parentEvent->contentJson().contains("via"_L1) && !parentEvent->contentPart<QJsonArray>("via"_L1).isEmpty()) {
            parentIds += parentEvent->stateKey();
        }
    }
    return parentIds;
}

QList<NeoChatRoom *> NeoChatRoom::parentObjects(bool multiLevel) const
{
    QList<NeoChatRoom *> parentObjects;
    QList<QString> parentIds = this->parentIds();
    for (const auto &parentId : parentIds) {
        if (auto parentObject = static_cast<NeoChatRoom *>(connection()->room(parentId))) {
            parentObjects += parentObject;
            if (multiLevel) {
                parentObjects += parentObject->parentObjects(true);
            }
        }
    }
    return parentObjects;
}

QString NeoChatRoom::canonicalParent() const
{
    auto parentEvents = currentState().eventsOfType("m.space.parent"_L1);
    for (const auto &parentEvent : parentEvents) {
        if (parentEvent->contentJson().contains("via"_L1) && !parentEvent->contentPart<QJsonArray>("via"_L1).isEmpty()) {
            if (parentEvent->contentPart<bool>("canonical"_L1)) {
                return parentEvent->stateKey();
            }
        }
    }
    return {};
}

void NeoChatRoom::setCanonicalParent(const QString &parentId)
{
    if (!canModifyParent(parentId)) {
        return;
    }
    if (const auto &parent = currentState().get("m.space.parent"_L1, parentId)) {
        auto content = parent->contentJson();
        content.insert("canonical"_L1, true);
        setState("m.space.parent"_L1, parentId, content);
    } else {
        return;
    }

    // Only one canonical parent can exist so make sure others are set false.
    auto parentEvents = currentState().eventsOfType("m.space.parent"_L1);
    for (const auto &parentEvent : parentEvents) {
        if (parentEvent->contentPart<bool>("canonical"_L1) && parentEvent->stateKey() != parentId) {
            auto content = parentEvent->contentJson();
            content.insert("canonical"_L1, false);
            setState("m.space.parent"_L1, parentEvent->stateKey(), content);
        }
    }
}

bool NeoChatRoom::canModifyParent(const QString &parentId) const
{
    if (!canSendState("m.space.parent"_L1)) {
        return false;
    }
    // If we can't peek the parent we assume that we neither have permission nor is
    // there an existing space child event for this room.
    if (auto parent = static_cast<NeoChatRoom *>(connection()->room(parentId))) {
        if (!parent->isSpace()) {
            return false;
        }
        // If the user is allowed to set space child events in the parent they are
        // allowed to set the space as a parent (even if a space child event doesn't
        // exist).
        if (parent->canSendState("m.space.child"_L1)) {
            return true;
        }
        // If the parent has a space child event the user can set as a parent (even
        // if they don't have permission to set space child events in that parent).
        if (parent->currentState().contains("m.space.child"_L1, id())) {
            return true;
        }
    }
    return false;
}

void NeoChatRoom::addParent(const QString &parentId, bool canonical, bool setParentChild)
{
    if (!canModifyParent(parentId)) {
        return;
    }
    if (canonical) {
        // Only one canonical parent can exist so make sure others are set false.
        auto parentEvents = currentState().eventsOfType("m.space.parent"_L1);
        for (const auto &parentEvent : parentEvents) {
            if (parentEvent->contentPart<bool>("canonical"_L1)) {
                auto content = parentEvent->contentJson();
                content.insert("canonical"_L1, false);
                setState("m.space.parent"_L1, parentEvent->stateKey(), content);
            }
        }
    }

    setState("m.space.parent"_L1, parentId, QJsonObject{{"canonical"_L1, canonical}, {"via"_L1, QJsonArray{connection()->domain()}}});

    if (setParentChild) {
        if (auto parent = static_cast<NeoChatRoom *>(connection()->room(parentId))) {
            parent->setState("m.space.child"_L1, id(), QJsonObject{{"via"_L1, QJsonArray{connection()->domain()}}});
        }
    }
}

void NeoChatRoom::removeParent(const QString &parentId)
{
    if (!canModifyParent(parentId)) {
        return;
    }
    if (!currentState().contains("m.space.parent"_L1, parentId)) {
        return;
    }
    setState("m.space.parent"_L1, parentId, {});
}

bool NeoChatRoom::isSpace() const
{
    const auto creationEvent = this->creation();
    if (!creationEvent) {
        return false;
    }

    return creationEvent->roomType() == RoomType::Space;
}

qsizetype NeoChatRoom::childrenNotificationCount()
{
    if (!isSpace()) {
        return 0;
    }
    return SpaceHierarchyCache::instance().notificationCountForSpace(id());
}

bool NeoChatRoom::childrenHaveHighlightNotifications() const
{
    if (!isSpace()) {
        return false;
    }
    return SpaceHierarchyCache::instance().spaceHasHighlightNotifications(id());
}

void NeoChatRoom::addChild(const QString &childId, bool setChildParent, bool canonical, bool suggested, const QString &order)
{
    if (!isSpace()) {
        return;
    }
    if (!canSendEvent("m.space.child"_L1)) {
        return;
    }
    setState("m.space.child"_L1, childId, QJsonObject{{"via"_L1, QJsonArray{connection()->domain()}}, {"suggested"_L1, suggested}, {"order"_L1, order}});

    if (setChildParent) {
        if (auto child = static_cast<NeoChatRoom *>(connection()->room(childId))) {
            if (child->canSendState("m.space.parent"_L1)) {
                child->setState("m.space.parent"_L1, id(), QJsonObject{{"canonical"_L1, canonical}, {"via"_L1, QJsonArray{connection()->domain()}}});

                if (canonical) {
                    // Only one canonical parent can exist so make sure others are set to false.
                    auto parentEvents = child->currentState().eventsOfType("m.space.parent"_L1);
                    for (const auto &parentEvent : parentEvents) {
                        if (parentEvent->contentPart<bool>("canonical"_L1)) {
                            auto content = parentEvent->contentJson();
                            content.insert("canonical"_L1, false);
                            setState("m.space.parent"_L1, parentEvent->stateKey(), content);
                        }
                    }
                }
            }
        }
    }
}

void NeoChatRoom::removeChild(const QString &childId, bool unsetChildParent)
{
    if (!isSpace()) {
        return;
    }
    if (!canSendEvent("m.space.child"_L1)) {
        return;
    }
    setState("m.space.child"_L1, childId, {});

    if (unsetChildParent) {
        if (auto child = static_cast<NeoChatRoom *>(connection()->room(childId))) {
            if (child->canSendState("m.space.parent"_L1) && child->currentState().contains("m.space.parent"_L1, id())) {
                child->setState("m.space.parent"_L1, id(), {});
            }
        }
    }
}

bool NeoChatRoom::isSuggested(const QString &childId)
{
    if (!currentState().contains("m.space.child"_L1, childId)) {
        return false;
    }
    const auto childEvent = currentState().get("m.space.child"_L1, childId);
    return childEvent->contentPart<bool>("suggested"_L1);
}

void NeoChatRoom::toggleChildSuggested(const QString &childId)
{
    if (!isSpace()) {
        return;
    }
    if (!canSendEvent("m.space.child"_L1)) {
        return;
    }
    if (const auto childEvent = currentState().get("m.space.child"_L1, childId)) {
        auto content = childEvent->contentJson();
        content.insert("suggested"_L1, !childEvent->contentPart<bool>("suggested"_L1));
        setState("m.space.child"_L1, childId, content);
    }
}

void NeoChatRoom::setChildOrder(const QString &childId, const QString &order)
{
    if (!isSpace()) {
        return;
    }
    if (!canSendEvent("m.space.child"_L1)) {
        return;
    }
    if (const auto childEvent = currentState().get("m.space.child"_L1, childId)) {
        auto content = childEvent->contentJson();
        if (!content.contains("via"_L1)) {
            return;
        }
        if (content.value("order"_L1).toString() == order) {
            return;
        }

        content.insert("order"_L1, order);
        setState("m.space.child"_L1, childId, content);
    }
}

PushNotificationState::State NeoChatRoom::pushNotificationState() const
{
    return m_currentPushNotificationState;
}

void NeoChatRoom::setPushNotificationState(PushNotificationState::State state)
{
    // The caller should never try to set the state to unknown.
    // It exists only as a default state to diable the settings options until the actual state is retrieved from the server.
    if (state == PushNotificationState::Unknown) {
        Q_ASSERT(false);
        return;
    }

    /**
     * This stops updatePushNotificationState from temporarily changing
     * m_pushNotificationStateUpdating to default after the exisitng rules are deleted but
     * before a new rule is added.
     * The value is set to false after the rule enable job is successful.
     */
    m_pushNotificationStateUpdating = true;

    /**
     * First remove any existing room rules of the wrong type.
     * Note to prevent race conditions any rule that is going ot be overridden later is not removed.
     * If the default push notification state is chosen any existing rule needs to be removed.
     */
    QJsonObject accountData = connection()->accountDataJson("m.push_rules"_L1);

    // For default and mute check for a room rule and remove if found.
    if (state == PushNotificationState::Default || state == PushNotificationState::Mute) {
        QJsonArray roomRuleArray = accountData["global"_L1].toObject()["room"_L1].toArray();
        for (const auto &i : roomRuleArray) {
            QJsonObject roomRule = i.toObject();
            if (roomRule["rule_id"_L1] == id()) {
                connection()->callApi<DeletePushRuleJob>("room"_L1, id());
            }
        }
    }

    // For default, all and @mentions and keywords check for an override rule and remove if found.
    if (state == PushNotificationState::Default || state == PushNotificationState::All || state == PushNotificationState::MentionKeyword) {
        QJsonArray overrideRuleArray = accountData["global"_L1].toObject()["override"_L1].toArray();
        for (const auto &i : overrideRuleArray) {
            QJsonObject overrideRule = i.toObject();
            if (overrideRule["rule_id"_L1] == id()) {
                connection()->callApi<DeletePushRuleJob>("override"_L1, id());
            }
        }
    }

    if (state == PushNotificationState::Mute) {
        /**
         * To mute a room an override rule with "don't notify is set".
         *
         * Setup the rule action to "don't notify" to stop all room notifications
         * see https://spec.matrix.org/v1.3/client-server-api/#actions
         *
         * "actions": [
         *      "don't_notify"
         * ]
         */
        const QList<QVariant> actions = {"dont_notify"_L1};
        /**
         * Setup the push condition to get all events for the current room
         * see https://spec.matrix.org/v1.3/client-server-api/#conditions-1
         *
         * "conditions": [
         *      {
         *          "key": "type",
         *          "kind": "event_match",
         *          "pattern": "room_id"
         *      }
         * ]
         */
        PushCondition pushCondition;
        pushCondition.kind = "event_match"_L1;
        pushCondition.key = "room_id"_L1;
        pushCondition.pattern = id();
        const QList<PushCondition> conditions = {pushCondition};

        // Add new override rule and make sure it's enabled
        auto job = connection()->callApi<SetPushRuleJob>("override"_L1, id(), actions, QString(), QString(), conditions, QString());
        connect(job, &BaseJob::success, this, [this]() {
            auto enableJob = connection()->callApi<SetPushRuleEnabledJob>("override"_L1, id(), true);
            connect(enableJob, &BaseJob::success, this, [this]() {
                m_pushNotificationStateUpdating = false;
            });
        });
    } else if (state == PushNotificationState::MentionKeyword) {
        /**
         * To only get notifcations for @ mentions and keywords a room rule with "don't_notify" is set.
         *
         * Note -  This works becuase a default override rule which catches all user mentions will
         * take precedent and notify. See https://spec.matrix.org/v1.3/client-server-api/#default-override-rules. Any keywords will also have a similar override
         * rule.
         *
         * Setup the rule action to "don't notify" to stop all room event notifications
         * see https://spec.matrix.org/v1.3/client-server-api/#actions
         *
         * "actions": [
         *      "don't_notify"
         * ]
         */
        const QList<QVariant> actions = {"dont_notify"_L1};
        // No conditions for a room rule
        const QList<PushCondition> conditions;

        auto setJob = connection()->callApi<SetPushRuleJob>("room"_L1, id(), actions, QString(), QString(), conditions, QString());
        connect(setJob, &BaseJob::success, this, [this]() {
            auto enableJob = connection()->callApi<SetPushRuleEnabledJob>("room"_L1, id(), true);
            connect(enableJob, &BaseJob::success, this, [this]() {
                m_pushNotificationStateUpdating = false;
            });
        });
    } else if (state == PushNotificationState::All) {
        /**
         * To send a notification for all room messages a room rule with "notify" is set.
         *
         * Setup the rule action to "notify" so all room events give notifications.
         * Tweeks is also set to follow default sound settings
         * see https://spec.matrix.org/v1.3/client-server-api/#actions
         *
         * "actions": [
         *      "notify",
         *      {
         *          "set_tweek": "sound",
         *          "value": "default",
         *      }
         * ]
         */
        QJsonObject tweaks;
        tweaks.insert("set_tweak"_L1, "sound"_L1);
        tweaks.insert("value"_L1, "default"_L1);
        const QList<QVariant> actions = {"notify"_L1, tweaks};
        // No conditions for a room rule
        const QList<PushCondition> conditions;

        // Add new room rule and make sure enabled
        auto setJob = connection()->callApi<SetPushRuleJob>("room"_L1, id(), actions, QString(), QString(), conditions, QString());
        connect(setJob, &BaseJob::success, this, [this]() {
            auto enableJob = connection()->callApi<SetPushRuleEnabledJob>("room"_L1, id(), true);
            connect(enableJob, &BaseJob::success, this, [this]() {
                m_pushNotificationStateUpdating = false;
            });
        });
    }

    m_currentPushNotificationState = state;
    Q_EMIT pushNotificationStateChanged(m_currentPushNotificationState);
}

void NeoChatRoom::updatePushNotificationState(QString type)
{
    if (type != "m.push_rules"_L1 || m_pushNotificationStateUpdating) {
        return;
    }

    QJsonObject accountData = connection()->accountDataJson("m.push_rules"_L1);

    // First look for a room rule with the room id
    QJsonArray roomRuleArray = accountData["global"_L1].toObject()["room"_L1].toArray();
    for (const auto &i : roomRuleArray) {
        QJsonObject roomRule = i.toObject();
        if (roomRule["rule_id"_L1] == id()) {
            if (roomRule["actions"_L1].toArray().size() == 0) {
                m_currentPushNotificationState = PushNotificationState::MentionKeyword;
                Q_EMIT pushNotificationStateChanged(m_currentPushNotificationState);
                return;
            }
            QString notifyAction = roomRule["actions"_L1].toArray()[0].toString();
            if (notifyAction == "notify"_L1) {
                m_currentPushNotificationState = PushNotificationState::All;
                Q_EMIT pushNotificationStateChanged(m_currentPushNotificationState);
                return;
            } else if (notifyAction == "dont_notify"_L1) {
                m_currentPushNotificationState = PushNotificationState::MentionKeyword;
                Q_EMIT pushNotificationStateChanged(m_currentPushNotificationState);
                return;
            }
        }
    }

    // Check for an override rule with the room id
    QJsonArray overrideRuleArray = accountData["global"_L1].toObject()["override"_L1].toArray();
    for (const auto &i : overrideRuleArray) {
        QJsonObject overrideRule = i.toObject();
        if (overrideRule["rule_id"_L1] == id()) {
            if (overrideRule["actions"_L1].toArray().isEmpty()) {
                m_currentPushNotificationState = PushNotificationState::Mute;
                Q_EMIT pushNotificationStateChanged(m_currentPushNotificationState);
                return;
            }
            QString notifyAction = overrideRule["actions"_L1].toArray()[0].toString();
            if (notifyAction == "dont_notify"_L1) {
                m_currentPushNotificationState = PushNotificationState::Mute;
                Q_EMIT pushNotificationStateChanged(m_currentPushNotificationState);
                return;
            }
        }
    }

    // If neither a room or override rule exist for the room then the setting must be default
    m_currentPushNotificationState = PushNotificationState::Default;
    Q_EMIT pushNotificationStateChanged(m_currentPushNotificationState);
}

void NeoChatRoom::reportEvent(const QString &eventId, const QString &reason)
{
    auto job = connection()->callApi<ReportContentJob>(id(), eventId, -50, reason);
    connect(job, &BaseJob::finished, this, [this, job]() {
        if (job->error() == BaseJob::Success) {
            Q_EMIT showMessage(MessageType::Positive, i18n("Report sent successfully."));
        }
    });
}

QByteArray NeoChatRoom::getEventJsonSource(const QString &eventId)
{
    auto evtIt = findInTimeline(eventId);
    if (evtIt != messageEvents().rend() && is<RoomEvent>(**evtIt)) {
        const auto event = evtIt->viewAs<RoomEvent>();
        return QJsonDocument(event->fullJson()).toJson();
    }
    return {};
}

void NeoChatRoom::openEventMediaExternally(const QString &eventId)
{
    const auto evtIt = findInTimeline(eventId);
    if (evtIt != messageEvents().rend() && is<RoomMessageEvent>(**evtIt)) {
        const auto event = evtIt->viewAs<RoomMessageEvent>();
        if (event->has<EventContent::FileContent>()) {
            const auto transferInfo = cachedFileTransferInfo(event);
            if (transferInfo.completed()) {
                UrlHelper helper;
                helper.openUrl(transferInfo.localPath);
            } else {
                downloadFile(eventId,
                             QUrl(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + u'/'
                                  + event->id().replace(u':', u'_').replace(u'/', u'_').replace(u'+', u'_') + fileNameToDownload(eventId)));
                connect(
                    this,
                    &Room::fileTransferCompleted,
                    this,
                    [this, eventId](QString id, QUrl localFile, FileSourceInfo fileMetadata) {
                        Q_UNUSED(localFile);
                        Q_UNUSED(fileMetadata);
                        if (id == eventId) {
                            auto transferInfo = fileTransferInfo(eventId);
                            UrlHelper helper;
                            helper.openUrl(transferInfo.localPath);
                        }
                    },
                    static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
            }
        }
    }
}

void NeoChatRoom::copyEventMedia(const QString &eventId)
{
    const auto evtIt = findInTimeline(eventId);
    if (evtIt != messageEvents().rend() && is<RoomMessageEvent>(**evtIt)) {
        const auto event = evtIt->viewAs<RoomMessageEvent>();
        if (event->has<EventContent::FileContent>()) {
            const auto transferInfo = fileTransferInfo(eventId);
            if (transferInfo.completed()) {
                Clipboard clipboard;
                clipboard.setImage(transferInfo.localPath);
            } else {
                downloadFile(eventId,
                             QUrl(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + u'/'
                                  + event->id().replace(u':', u'_').replace(u'/', u'_').replace(u'+', u'_') + fileNameToDownload(eventId)));
                connect(
                    this,
                    &Room::fileTransferCompleted,
                    this,
                    [this, eventId](QString id, QUrl localFile, FileSourceInfo fileMetadata) {
                        Q_UNUSED(localFile);
                        Q_UNUSED(fileMetadata);
                        if (id == eventId) {
                            auto transferInfo = fileTransferInfo(eventId);
                            Clipboard clipboard;
                            clipboard.setImage(transferInfo.localPath);
                        }
                    },
                    static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
            }
        }
    }
}

FileTransferInfo NeoChatRoom::cachedFileTransferInfo(const Quotient::RoomEvent *event) const
{
    QString mxcUrl;
    int total = 0;
    if (auto evt = eventCast<const Quotient::RoomMessageEvent>(event)) {
        if (evt->has<EventContent::FileContent>()) {
            const auto fileContent = evt->get<EventContent::FileContent>();

            mxcUrl = fileContent->url().toString();
            total = fileContent->payloadSize;
        }
    } else if (auto evt = eventCast<const Quotient::StickerEvent>(event)) {
        mxcUrl = evt->image().url().toString();
        total = evt->image().payloadSize;
    }

    FileTransferInfo transferInfo = fileTransferInfo(event->id());
    if (transferInfo.active()) {
        return transferInfo;
    }

    auto config = KSharedConfig::openStateConfig(u"neochatdownloads"_s)->group(u"downloads"_s);
    if (!config.hasKey(mxcUrl.mid(6))) {
        return transferInfo;
    }

    const auto path = config.readPathEntry(mxcUrl.mid(6), QString());
    QFileInfo info(path);
    if (!info.isFile()) {
        config.deleteEntry(mxcUrl);
        return transferInfo;
    }
    // TODO: we could check the hash here
    return FileTransferInfo{
        .status = FileTransferInfo::Completed,
        .isUpload = false,
        .progress = total,
        .total = total,
        .localDir = QUrl(info.dir().path()),
        .localPath = QUrl::fromLocalFile(path),
    };
}

ChatBarCache *NeoChatRoom::mainCache() const
{
    return m_mainCache;
}

ChatBarCache *NeoChatRoom::editCache() const
{
    return m_editCache;
}

ChatBarCache *NeoChatRoom::threadCache() const
{
    return m_threadCache;
}

void NeoChatRoom::replyLastMessage()
{
    const auto &timelineBottom = messageEvents().rbegin();

    // set a cap limit of startRow + 35 messages, to prevent loading a lot of messages
    // in rooms where the user has not sent many messages
    const auto limit = timelineBottom + std::min(35, timelineSize());

    for (auto it = timelineBottom; it != limit; ++it) {
        auto evt = it->event();
        auto e = eventCast<const RoomMessageEvent>(evt);
        if (!e) {
            continue;
        }

        auto content = (*it)->contentJson();

        if (e->msgtype() != MessageEventType::Unknown) {
            QString eventId;
            if (content.contains("m.new_content"_L1)) {
                // The message has been edited so we have to return the id of the original message instead of the replacement
                eventId = content["m.relates_to"_L1].toObject()["event_id"_L1].toString();
            } else {
                // For any message that isn't an edit return the id of the current message
                eventId = (*it)->id();
            }
            mainCache()->setReplyId(eventId);
            return;
        }
    }
}

void NeoChatRoom::editLastMessage()
{
    const auto &timelineBottom = messageEvents().rbegin();

    // set a cap limit of 35 messages, to prevent loading a lot of messages
    // in rooms where the user has not sent many messages
    const auto limit = timelineBottom + std::min(35, timelineSize());

    for (auto it = timelineBottom; it != limit; ++it) {
        auto evt = it->event();
        auto e = eventCast<const RoomMessageEvent>(evt);
        if (!e) {
            continue;
        }

        // check if the current message's sender's id is same as the user's id
        if ((*it)->senderId() == localMember().id()) {
            auto content = (*it)->contentJson();

            if (e->msgtype() != MessageEventType::Unknown) {
                QString eventId;
                if (content.contains("m.new_content"_L1)) {
                    // The message has been edited so we have to return the id of the original message instead of the replacement
                    eventId = content["m.relates_to"_L1].toObject()["event_id"_L1].toString();
                } else {
                    // For any message that isn't an edit return the id of the current message
                    eventId = (*it)->id();
                }
                editCache()->setEditId(eventId);
                return;
            }
        }
    }
}

bool NeoChatRoom::canEncryptRoom() const
{
    return !usesEncryption() && canSendState("m.room.encryption"_L1);
}

static PollHandler *emptyPollHandler = new PollHandler;

PollHandler *NeoChatRoom::poll(const QString &eventId) const
{
    if (auto pollHandler = m_polls[eventId]) {
        return pollHandler;
    }
    return emptyPollHandler;
}

void NeoChatRoom::createPollHandler(const Quotient::PollStartEvent *event)
{
    if (event == nullptr) {
        return;
    }
    auto eventId = event->id();
    if (!m_polls.contains(eventId)) {
        auto handler = new PollHandler(this, event);
        m_polls.insert(eventId, handler);
    }
}

bool NeoChatRoom::downloadTempFile(const QString &eventId)
{
    QTemporaryFile file;
    file.setAutoRemove(false);
    if (!file.open()) {
        return false;
    }

    download(eventId, QUrl::fromLocalFile(file.fileName()));
    return true;
}

void NeoChatRoom::download(const QString &eventId, const QUrl &localFilename)
{
    downloadFile(eventId, localFilename);
#ifndef Q_OS_ANDROID
    auto job = new FileTransferPseudoJob(FileTransferPseudoJob::Download, localFilename.toLocalFile(), eventId);
    connect(this, &Room::fileTransferProgress, job, &FileTransferPseudoJob::fileTransferProgress);
    connect(this, &Room::fileTransferCompleted, job, &FileTransferPseudoJob::fileTransferCompleted);
    connect(this, &Room::fileTransferFailed, job, [this, job, eventId] {
        auto info = fileTransferInfo(eventId);
        if (info.status == FileTransferInfo::Cancelled) {
            job->fileTransferCanceled(eventId);
        } else {
            job->fileTransferFailed(eventId);
        }
    });
    connect(job, &FileTransferPseudoJob::cancelRequested, this, &Room::cancelFileTransfer);
    KIO::getJobTracker()->registerJob(job);
    job->start();
#endif
}

void NeoChatRoom::mapAlias(const QString &alias)
{
    auto getLocalAliasesJob = connection()->callApi<GetLocalAliasesJob>(id());
    connect(getLocalAliasesJob, &BaseJob::success, this, [this, getLocalAliasesJob, alias] {
        if (getLocalAliasesJob->aliases().contains(alias)) {
            return;
        } else {
            auto setRoomAliasJob = connection()->callApi<SetRoomAliasJob>(alias, id());
            connect(setRoomAliasJob, &BaseJob::success, this, [this, alias] {
                auto newAltAliases = altAliases();
                newAltAliases.append(alias);
                setLocalAliases(newAltAliases);
            });
        }
    });
}

void NeoChatRoom::unmapAlias(const QString &alias)
{
    connection()->callApi<DeleteRoomAliasJob>(alias);
}

void NeoChatRoom::setCanonicalAlias(const QString &newAlias)
{
    QString oldCanonicalAlias = canonicalAlias();
    Room::setCanonicalAlias(newAlias);

    connect(this, &Room::namesChanged, this, [this, newAlias, oldCanonicalAlias] {
        if (canonicalAlias() == newAlias) {
            // If the new canonical alias is already a published alt alias remove it otherwise it will be in both lists.
            // The server doesn't prevent this so we need to handle it.
            auto newAltAliases = altAliases();
            if (!oldCanonicalAlias.isEmpty()) {
                newAltAliases.append(oldCanonicalAlias);
            }
            if (newAltAliases.contains(newAlias)) {
                newAltAliases.removeAll(newAlias);
                Room::setLocalAliases(newAltAliases);
            }
        }
    });
}

int NeoChatRoom::maxRoomVersion() const
{
    int maxVersion = 0;
    for (auto roomVersion : connection()->availableRoomVersions()) {
        if (roomVersion.id.toInt() > maxVersion) {
            maxVersion = roomVersion.id.toInt();
        }
    }
    return maxVersion;
}

NeochatRoomMember *NeoChatRoom::directChatRemoteMember()
{
    if (directChatMembers().size() == 0) {
        qWarning() << "No other member available in this room";
        return {};
    }
    return new NeochatRoomMember(this, directChatMembers()[0].id());
}

void NeoChatRoom::sendLocation(float lat, float lon, const QString &description)
{
    QJsonObject locationContent{
        {"uri"_L1, "geo:%1,%2"_L1.arg(QString::number(lat), QString::number(lon))},
    };

    if (!description.isEmpty()) {
        locationContent["description"_L1] = description;
    }

    QJsonObject content{
        {"body"_L1, i18nc("'Lat' and 'Lon' as in Latitude and Longitude", "Lat: %1, Lon: %2", lat, lon)},
        {"msgtype"_L1, "m.location"_L1},
        {"geo_uri"_L1, "geo:%1,%2"_L1.arg(QString::number(lat), QString::number(lon))},
        {"org.matrix.msc3488.location"_L1, locationContent},
        {"org.matrix.msc3488.asset"_L1,
         QJsonObject{
             {"type"_L1, "m.pin"_L1},
         }},
        {"org.matrix.msc1767.text"_L1, i18nc("'Lat' and 'Lon' as in Latitude and Longitude", "Lat: %1, Lon: %2", lat, lon)},
    };
    postJson("m.room.message"_L1, content);
}

QByteArray NeoChatRoom::roomAcountDataJson(const QString &eventType)
{
    return QJsonDocument(accountData(eventType)->fullJson()).toJson();
}

void NeoChatRoom::downloadEventFromServer(const QString &eventId)
{
    if (findInTimeline(eventId) != historyEdge()) {
        // For whatever reason the event has now appeared so the function that called
        // this need to whatever it wanted to do with the event.
        Q_EMIT extraEventLoaded(eventId);
        return;
    }
    auto job = connection()->callApi<GetOneRoomEventJob>(id(), eventId);
    connect(job, &BaseJob::success, this, [this, job, eventId] {
        // The event may have arrived in the meantime so check it's not in the timeline.
        if (findInTimeline(eventId) != historyEdge()) {
            Q_EMIT extraEventLoaded(eventId);
            return;
        }

        event_ptr_tt<RoomEvent> event = fromJson<event_ptr_tt<RoomEvent>>(job->jsonData());
        m_extraEvents.push_back(std::move(event));
        Q_EMIT extraEventLoaded(eventId);
    });
    connect(job, &BaseJob::failure, this, [this, job, eventId] {
        if (job->error() == BaseJob::NotFound) {
            Q_EMIT extraEventNotFound(eventId);
        }
    });
}

std::pair<const Quotient::RoomEvent *, bool> NeoChatRoom::getEvent(const QString &eventId) const
{
    if (eventId.isEmpty()) {
        return {};
    }
    const auto timelineIt = findInTimeline(eventId);
    if (timelineIt != historyEdge()) {
        return std::make_pair(timelineIt->get(), false);
    }

    auto pendingIt = findPendingEvent(eventId);
    if (pendingIt != pendingEvents().end()) {
        return std::make_pair(pendingIt->event(), true);
    }
    // findPendingEvent() searches by transaction ID, we also need to check event ID.
    for (const auto &event : pendingEvents()) {
        if (event->id() == eventId || event->transactionId() == eventId) {
            return std::make_pair(event.event(), true);
        }
    }

    auto extraIt = std::find_if(m_extraEvents.begin(), m_extraEvents.end(), [eventId](const Quotient::event_ptr_tt<Quotient::RoomEvent> &event) {
        return event->id() == eventId;
    });
    return std::make_pair(extraIt != m_extraEvents.end() ? extraIt->get() : nullptr, false);
}

const RoomEvent *NeoChatRoom::getReplyForEvent(const RoomEvent &event) const
{
    const QString &replyEventId = event.contentJson()["m.relates_to"_L1].toObject()["m.in_reply_to"_L1].toObject()["event_id"_L1].toString();
    if (replyEventId.isEmpty()) {
        return {};
    };

    const auto replyIt = findInTimeline(replyEventId);
    const RoomEvent *replyPtr = replyIt != historyEdge() ? &**replyIt : nullptr;
    if (!replyPtr) {
        for (const auto &e : m_extraEvents) {
            if (e->id() == replyEventId) {
                replyPtr = e.get();
                break;
            }
        }
    }
    return replyPtr;
}

void NeoChatRoom::cleanupExtraEventRange(Quotient::RoomEventsRange events)
{
    for (auto &&event : events) {
        cleanupExtraEvent(event->id());
    }
}

void NeoChatRoom::cleanupExtraEvent(const QString &eventId)
{
    auto it = std::find_if(m_extraEvents.begin(), m_extraEvents.end(), [eventId](Quotient::event_ptr_tt<Quotient::RoomEvent> &event) {
        return event->id() == eventId;
    });

    if (it != m_extraEvents.end()) {
        m_extraEvents.erase(it);
    }
}
QString NeoChatRoom::invitingUserId() const
{
    return currentState().get<RoomMemberEvent>(connection()->userId())->senderId();
}

void NeoChatRoom::setRoomState(const QString &type, const QString &stateKey, const QByteArray &content)
{
    setState(type, stateKey, QJsonDocument::fromJson(content).object());
}

NeochatRoomMember *NeoChatRoom::qmlSafeMember(const QString &memberId)
{
    if (!m_memberObjects.contains(memberId)) {
        return m_memberObjects.emplace(memberId, std::make_unique<NeochatRoomMember>(this, memberId)).first->second.get();
    }

    return m_memberObjects[memberId].get();
}

MessageContentModel *NeoChatRoom::contentModelForEvent(const QString &eventId)
{
    if (eventId.isEmpty()) {
        return nullptr;
    }

    if (!m_eventContentModels.contains(eventId)) {
        return m_eventContentModels.emplace(eventId, std::make_unique<MessageContentModel>(this, eventId)).first->second.get();
    }

    return m_eventContentModels[eventId].get();
}

MessageContentModel *NeoChatRoom::contentModelForEvent(const Quotient::RoomEvent *event)
{
    const auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(event);
    if (roomMessageEvent == nullptr) {
        // If for some reason a model is there remove.
        if (m_eventContentModels.contains(event->id())) {
            m_eventContentModels.erase(event->id());
        }
        if (m_eventContentModels.contains(event->transactionId())) {
            m_eventContentModels.erase(event->transactionId());
        }
        return nullptr;
    }

    if (event->isStateEvent() || event->matrixType() == u"org.matrix.msc3672.beacon_info"_s) {
        return nullptr;
    }

    auto eventId = event->id();
    const auto txnId = event->transactionId();
    if (!m_eventContentModels.contains(eventId) && !m_eventContentModels.contains(txnId)) {
        return m_eventContentModels
            .emplace(eventId.isEmpty() ? txnId : eventId,
                     std::make_unique<MessageContentModel>(this, eventId.isEmpty() ? txnId : eventId, false, eventId.isEmpty()))
            .first->second.get();
    }

    if (!eventId.isEmpty() && m_eventContentModels.contains(eventId)) {
        return m_eventContentModels[eventId].get();
    }

    if (!txnId.isEmpty() && m_eventContentModels.contains(txnId)) {
        if (eventId.isEmpty()) {
            return m_eventContentModels[txnId].get();
        }

        // If we now have an event ID use that as the map key instead of transaction ID.
        auto txnModel = std::move(m_eventContentModels[txnId]);
        m_eventContentModels.erase(txnId);
        return m_eventContentModels.emplace(eventId, std::move(txnModel)).first->second.get();
    }

    return nullptr;
}

ThreadModel *NeoChatRoom::modelForThread(const QString &threadRootId)
{
    if (threadRootId.isEmpty()) {
        return nullptr;
    }

    if (!m_threadModels.contains(threadRootId)) {
        return m_threadModels.emplace(threadRootId, std::make_unique<ThreadModel>(threadRootId, this)).first->second.get();
    }

    return m_threadModels[threadRootId].get();
}

void NeoChatRoom::pinEvent(const QString &eventId)
{
    auto eventIds = pinnedEventIds();
    eventIds.push_back(eventId);
    setPinnedEvents(eventIds);
}

void NeoChatRoom::unpinEvent(const QString &eventId)
{
    auto eventIds = pinnedEventIds();
    eventIds.removeAll(eventId);
    setPinnedEvents(eventIds);
}

bool NeoChatRoom::isEventPinned(const QString &eventId) const
{
    return pinnedEventIds().contains(eventId);
}

#include "moc_neochatroom.cpp"
