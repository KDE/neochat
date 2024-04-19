// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "neochatroom.h"

#include <QFileInfo>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QMimeDatabase>
#include <QTemporaryFile>

#include <Quotient/jobs/basejob.h>
#include <Quotient/quotient_common.h>
#include <Quotient/user.h>
#include <qcoro/qcorosignal.h>

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
#include "events/joinrulesevent.h"
#include "events/pollevent.h"
#include "filetransferpseudojob.h"
#include "neochatconfig.h"
#include "notificationsmanager.h"
#include "roomlastmessageprovider.h"
#include "spacehierarchycache.h"
#include "texthandler.h"
#include "urlhelper.h"
#include "utils.h"

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

    connect(connection, &Connection::accountDataChanged, this, &NeoChatRoom::updatePushNotificationState);
    connect(this, &Room::fileTransferCompleted, this, [this] {
        setFileUploadingProgress(0);
        setHasFileUploading(false);
    });

    connect(this, &Room::aboutToAddHistoricalMessages, this, &NeoChatRoom::readMarkerLoadedChanged);

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

    connectSingleShot(this, &Room::baseStateLoaded, this, [this]() {
        updatePushNotificationState(QStringLiteral("m.push_rules"));

        Q_EMIT canEncryptRoomChanged();
        if (this->joinState() != JoinState::Invite) {
            return;
        }
        auto roomMemberEvent = currentState().get<RoomMemberEvent>(localMember().id());
        QImage avatar_image;
        if (roomMemberEvent && !member(roomMemberEvent->senderId()).avatarUrl().isEmpty()) {
            // TODO: Fix acessing Avatar Image.
            // avatar_image = member(roomMemberEvent->senderId()).avatarMediaId();
            avatar_image = avatar(128);
        } else {
            qWarning() << "using this room's avatar";
            avatar_image = avatar(128);
        }
        NotificationsManager::instance().postInviteNotification(this, displayName(), member(roomMemberEvent->senderId()).htmlSafeDisplayName(), avatar_image);
    });
    connect(this, &Room::changed, this, [this] {
        Q_EMIT canEncryptRoomChanged();
        Q_EMIT parentIdsChanged();
        Q_EMIT canonicalParentChanged();
        Q_EMIT joinRuleChanged();
        Q_EMIT readOnlyChanged();
    });
    connect(connection, &Connection::capabilitiesLoaded, this, &NeoChatRoom::maxRoomVersionChanged);
    connect(this, &Room::changed, this, [this]() {
        Q_EMIT defaultUrlPreviewStateChanged();
    });
    connect(this, &Room::accountDataChanged, this, [this](QString type) {
        if (type == "org.matrix.room.preview_urls"_ls) {
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
    url.setScheme("file"_ls);
    QFileInfo fileInfo(url.isLocalFile() ? url.toLocalFile() : url.toString());
    EventContent::TypedBase *content;
    if (mime.name().startsWith("image/"_ls)) {
        QImage image(url.toLocalFile());
        content = new EventContent::ImageContent(url, fileInfo.size(), mime, image.size(), fileInfo.fileName());
    } else if (mime.name().startsWith("audio/"_ls)) {
        content = new EventContent::AudioContent(url, fileInfo.size(), mime, fileInfo.fileName());
    } else if (mime.name().startsWith("video/"_ls)) {
        QMediaPlayer player;
        player.setSource(url);
        co_await qCoro(&player, &QMediaPlayer::mediaStatusChanged);
        auto resolution = player.metaData().value(QMediaMetaData::Resolution).toSize();
        content = new EventContent::VideoContent(url, fileInfo.size(), mime, resolution, fileInfo.fileName());
    } else {
        content = new EventContent::FileContent(url, fileInfo.size(), mime, fileInfo.fileName());
    }
    QString txnId = postFile(body.isEmpty() ? url.fileName() : body, content);
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
    connect(this, &Room::fileTransferFailed, job, &FileTransferPseudoJob::fileTransferFailed);
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
    if (const auto &predecessor = dynamic_cast<NeoChatRoom *>(this->predecessor(JoinState::Join))) {
        predecessor->forget();
    }
    connection()->forgetRoom(id());
}

QVariantList NeoChatRoom::getUsersTyping() const
{
    auto users = usersTyping();
    users.removeAll(localMember());
    QVariantList userVariants;
    for (const auto &user : users) {
        if (connection()->isIgnored(user->id())) {
            continue;
        }
        userVariants.append(QVariantMap{
            {"id"_ls, user->id()},
            {"avatarMediaId"_ls, user->avatarMediaId(this)},
            {"displayName"_ls, user->displayname()},
            {"display"_ls, user->name()},
        });
    }
    return userVariants;
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

        if (connection()->isIgnored(user(event->senderId()))) {
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
            if (e->hasTextContent() && e->content() && e->mimeType().name() == "text/html"_ls) {
                auto htmlBody = static_cast<const Quotient::EventContent::TextContent *>(e->content())->body;
                return htmlBody.contains("data-mx-spoiler"_ls);
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
    auto localMemberId = localMember().id();
    if (ti->senderId() == localMemberId) {
        return;
    }
    if (auto *e = ti.viewAs<RoomMessageEvent>()) {
        const auto &text = e->plainBody();
        if (text.contains(localMemberId) || text.contains(safeMemberName(localMemberId))) {
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

// An empty user is useful for returning as a model value to avoid properties being undefined.
static const QVariantMap emptyUser = {
    {"isLocalUser"_ls, false},
    {"id"_ls, QString()},
    {"displayName"_ls, QString()},
    {"avatarSource"_ls, QUrl()},
    {"avatarMediaId"_ls, QString()},
    {"color"_ls, QColor()},
    {"object"_ls, QVariant()},
};

QVariantMap NeoChatRoom::getUser(const QString &userID) const
{
    return getUser(user(userID));
}

QVariantMap NeoChatRoom::getUser(User *user) const
{
    if (user == nullptr) {
        return emptyUser;
    }

    return QVariantMap{
        {QStringLiteral("isLocalUser"), user->id() == localMember().id()},
        {QStringLiteral("id"), user->id()},
        {QStringLiteral("displayName"), user->displayname()},
        {QStringLiteral("escapedDisplayName"), member(user->id()).htmlSafeDisplayName()},
        {QStringLiteral("avatarSource"), avatarForMember(user)},
        {QStringLiteral("avatarMediaId"), user->avatarMediaId()},
        {QStringLiteral("color"), member(user->id()).hueF()},
        {QStringLiteral("object"), QVariant::fromValue(user)},
    };
}

QString NeoChatRoom::avatarMediaId() const
{
    if (const auto avatar = Room::avatarMediaId(); !avatar.isEmpty()) {
        return avatar;
    }

    // Use the first (excluding self) user's avatar for direct chats
    const auto dcUsers = directChatMembers();
    for (const auto u : dcUsers) {
        if (u != localMember()) {
            return u->avatarMediaId(this);
        }
    }

    return {};
}

void NeoChatRoom::changeAvatar(const QUrl &localFile)
{
    const auto job = connection()->uploadFile(localFile.toLocalFile());
    if (isJobPending(job)) {
        connect(job, &BaseJob::success, this, [this, job] {
            connection()->callApi<SetRoomStateWithKeyJob>(id(), "m.room.avatar"_ls, QString(), QJsonObject{{"url"_ls, job->contentUri().toString()}});
        });
    }
}

QString msgTypeToString(MessageEventType msgType)
{
    switch (msgType) {
    case MessageEventType::Text:
        return "m.text"_ls;
    case MessageEventType::File:
        return "m.file"_ls;
    case MessageEventType::Audio:
        return "m.audio"_ls;
    case MessageEventType::Emote:
        return "m.emote"_ls;
    case MessageEventType::Image:
        return "m.image"_ls;
    case MessageEventType::Video:
        return "m.video"_ls;
    case MessageEventType::Notice:
        return "m.notice"_ls;
    case MessageEventType::Location:
        return "m.location"_ls;
    default:
        return "m.text"_ls;
    }
}

void NeoChatRoom::postMessage(const QString &rawText,
                              const QString &text,
                              MessageEventType type,
                              const QString &replyEventId,
                              const QString &relateToEventId,
                              const QString &threadRootId)
{
    postHtmlMessage(rawText, text, type, replyEventId, relateToEventId, threadRootId);
}

void NeoChatRoom::postHtmlMessage(const QString &text,
                                  const QString &html,
                                  MessageEventType type,
                                  const QString &replyEventId,
                                  const QString &relateToEventId,
                                  const QString &threadRootId)
{
    bool isReply = !replyEventId.isEmpty();
    bool isEdit = !relateToEventId.isEmpty();
    bool isThread = !threadRootId.isEmpty();
    const auto replyIt = findInTimeline(replyEventId);
    if (replyIt == historyEdge()) {
        isReply = false;
    }

    if (isThread) {
        EventHandler eventHandler(this, &**replyIt);

        const bool isFallingBack = !eventHandler.isThreaded();

        // clang-format off
        QJsonObject json{
          {"msgtype"_ls, msgTypeToString(type)},
          {"body"_ls, text},
          {"format"_ls, "org.matrix.custom.html"_ls},
          {"m.relates_to"_ls,
            QJsonObject {
              {"rel_type"_ls, "m.thread"_ls},
              {"event_id"_ls, threadRootId},
              {"is_falling_back"_ls, isFallingBack},
              {"m.in_reply_to"_ls,
                QJsonObject {
                  {"event_id"_ls, replyEventId}
                }
              }
            }
          },
          {"formatted_body"_ls, html}
        };
        // clang-format on

        postJson("m.room.message"_ls, json);
        return;
    }

    if (isEdit) {
        QJsonObject json{
            {"type"_ls, "m.room.message"_ls},
            {"msgtype"_ls, msgTypeToString(type)},
            {"body"_ls, "* %1"_ls.arg(text)},
            {"format"_ls, "org.matrix.custom.html"_ls},
            {"formatted_body"_ls, html},
            {"m.new_content"_ls,
             QJsonObject{{"body"_ls, text}, {"msgtype"_ls, msgTypeToString(type)}, {"format"_ls, "org.matrix.custom.html"_ls}, {"formatted_body"_ls, html}}},
            {"m.relates_to"_ls, QJsonObject{{"rel_type"_ls, "m.replace"_ls}, {"event_id"_ls, relateToEventId}}}};

        postJson("m.room.message"_ls, json);
        return;
    }

    if (isReply) {
        const auto &replyEvt = **replyIt;

        EventHandler eventHandler(this, &**replyIt);

        // clang-format off
        QJsonObject json{
          {"msgtype"_ls, msgTypeToString(type)},
          {"body"_ls, "> <%1> %2\n\n%3"_ls.arg(replyEvt.senderId(), eventHandler.getPlainBody(), text)},
          {"format"_ls, "org.matrix.custom.html"_ls},
          {"m.relates_to"_ls,
            QJsonObject {
              {"m.in_reply_to"_ls,
                QJsonObject {
                  {"event_id"_ls, replyEventId}
                }
              }
            }
          },
          {"formatted_body"_ls,
              "<mx-reply><blockquote><a href=\"https://matrix.to/#/%1/%2\">In reply to</a> <a href=\"https://matrix.to/#/%3\">%4</a><br>%5</blockquote></mx-reply>%6"_ls.arg(id(), replyEventId, replyEvt.senderId(), replyEvt.senderId(), eventHandler.getRichBody(), html)
          }
        };
        // clang-format on

        postJson("m.room.message"_ls, json);

        return;
    }

    Room::postHtmlMessage(text, html, type);
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
    return !canSendEvent("m.room.message"_ls);
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

QString NeoChatRoom::joinRule() const
{
    auto joinRulesEvent = currentState().get<JoinRulesEvent>();
    if (!joinRulesEvent) {
        return {};
    }
    return joinRulesEvent->joinRule();
}

void NeoChatRoom::setJoinRule(const QString &joinRule, const QList<QString> &allowedSpaces)
{
    if (!canSendState("m.room.join_rules"_ls)) {
        qWarning() << "Power level too low to set join rules";
        return;
    }
    auto actualRule = joinRule;
    if (joinRule == "restricted"_ls && allowedSpaces.isEmpty()) {
        actualRule = "private"_ls;
    }

    QJsonArray allowConditions;
    if (actualRule == "restricted"_ls) {
        for (auto allowedSpace : allowedSpaces) {
            allowConditions += QJsonObject{{"type"_ls, "m.room_membership"_ls}, {"room_id"_ls, allowedSpace}};
        }
    }

    QJsonObject content;
    content.insert("join_rule"_ls, joinRule);
    if (!allowConditions.isEmpty()) {
        content.insert("allow"_ls, allowConditions);
    }
    qWarning() << content;
    setState("m.room.join_rules"_ls, {}, content);
    // Not emitting joinRuleChanged() here, since that would override the change in the UI with the *current* value, which is not the *new* value.
}

QList<QString> NeoChatRoom::restrictedIds() const
{
    auto joinRulesEvent = currentState().get<JoinRulesEvent>();
    if (!joinRulesEvent) {
        return {};
    }
    if (joinRulesEvent->joinRule() != "restricted"_ls) {
        return {};
    }

    QList<QString> roomIds;
    for (auto allow : joinRulesEvent->allow()) {
        roomIds += allow.toObject().value("room_id"_ls).toString();
    }
    return roomIds;
}

QString NeoChatRoom::historyVisibility() const
{
    return currentState().get("m.room.history_visibility"_ls)->contentJson()["history_visibility"_ls].toString();
}

void NeoChatRoom::setHistoryVisibility(const QString &historyVisibilityRule)
{
    if (!canSendState("m.room.history_visibility"_ls)) {
        qWarning() << "Power level too low to set history visibility";
        return;
    }

    setState("m.room.history_visibility"_ls, {}, QJsonObject{{"history_visibility"_ls, historyVisibilityRule}});
    // Not emitting historyVisibilityChanged() here, since that would override the change in the UI with the *current* value, which is not the *new* value.
}

bool NeoChatRoom::defaultUrlPreviewState() const
{
    auto urlPreviewsDisabled = currentState().get("org.matrix.room.preview_urls"_ls);

    // Some rooms will not have this state event set so check for a nullptr return.
    if (urlPreviewsDisabled != nullptr) {
        return !urlPreviewsDisabled->contentJson()["disable"_ls].toBool();
    } else {
        return false;
    }
}

void NeoChatRoom::setDefaultUrlPreviewState(const bool &defaultUrlPreviewState)
{
    if (!canSendState("org.matrix.room.preview_urls"_ls)) {
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
    setState("org.matrix.room.preview_urls"_ls, {}, QJsonObject{{"disable"_ls, !defaultUrlPreviewState}});
}

bool NeoChatRoom::urlPreviewEnabled() const
{
    if (hasAccountData("org.matrix.room.preview_urls"_ls)) {
        return !accountData("org.matrix.room.preview_urls"_ls)->contentJson()["disable"_ls].toBool();
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
                                                    "org.matrix.room.preview_urls"_ls,
                                                    QJsonObject{{"disable"_ls, !urlPreviewEnabled}});
}

void NeoChatRoom::setUserPowerLevel(const QString &userID, const int &powerLevel)
{
    if (joinedCount() <= 1) {
        qWarning() << "Cannot modify the power level of the only user";
        return;
    }
    if (!canSendState("m.room.power_levels"_ls)) {
        qWarning() << "Power level too low to set user power levels";
        return;
    }
    if (!isMember(userID)) {
        qWarning() << "User is not a member of this room so power level cannot be set";
        return;
    }
    int clampPowerLevel = std::clamp(powerLevel, 0, 100);

    auto powerLevelContent = currentState().get("m.room.power_levels"_ls)->contentJson();
    auto powerLevelUserOverrides = powerLevelContent["users"_ls].toObject();

    if (powerLevelUserOverrides[userID] != clampPowerLevel) {
        powerLevelUserOverrides[userID] = clampPowerLevel;
        powerLevelContent["users"_ls] = powerLevelUserOverrides;

        setState("m.room.power_levels"_ls, {}, powerLevelContent);
    }
}

int NeoChatRoom::getUserPowerLevel(const QString &userId) const
{
    auto powerLevelEvent = currentState().get<RoomPowerLevelsEvent>();
    if (!powerLevelEvent) {
        return 0;
    }
    return powerLevelEvent->powerLevelForUser(userId);
}

int NeoChatRoom::powerLevel(const QString &eventName, const bool &isStateEvent) const
{
    const auto powerLevelEvent = currentState().get<RoomPowerLevelsEvent>();
    if (eventName == "ban"_ls) {
        return powerLevelEvent->ban();
    } else if (eventName == "kick"_ls) {
        return powerLevelEvent->kick();
    } else if (eventName == "invite"_ls) {
        return powerLevelEvent->invite();
    } else if (eventName == "redact"_ls) {
        return powerLevelEvent->redact();
    } else if (eventName == "users_default"_ls) {
        return powerLevelEvent->usersDefault();
    } else if (eventName == "state_default"_ls) {
        return powerLevelEvent->stateDefault();
    } else if (eventName == "events_default"_ls) {
        return powerLevelEvent->eventsDefault();
    } else if (isStateEvent) {
        return powerLevelEvent->powerLevelForState(eventName);
    } else {
        return powerLevelEvent->powerLevelForEvent(eventName);
    }
}

void NeoChatRoom::setPowerLevel(const QString &eventName, const int &newPowerLevel, const bool &isStateEvent)
{
    auto powerLevelContent = currentState().get("m.room.power_levels"_ls)->contentJson();
    int clampPowerLevel = std::clamp(newPowerLevel, 0, 100);
    int powerLevel = 0;

    if (powerLevelContent.contains(eventName)) {
        powerLevel = powerLevelContent[eventName].toInt();

        if (powerLevel != clampPowerLevel) {
            powerLevelContent[eventName] = clampPowerLevel;
        }
    } else {
        auto eventPowerLevels = powerLevelContent["events"_ls].toObject();

        if (eventPowerLevels.contains(eventName)) {
            powerLevel = eventPowerLevels[eventName].toInt();
        } else {
            if (isStateEvent) {
                powerLevel = powerLevelContent["state_default"_ls].toInt();
            } else {
                powerLevel = powerLevelContent["events_default"_ls].toInt();
            }
        }

        if (powerLevel != clampPowerLevel) {
            eventPowerLevels[eventName] = clampPowerLevel;
            powerLevelContent["events"_ls] = eventPowerLevels;
        }
    }

    setState("m.room.power_levels"_ls, {}, powerLevelContent);
}

int NeoChatRoom::defaultUserPowerLevel() const
{
    return powerLevel("users_default"_ls);
}

void NeoChatRoom::setDefaultUserPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("users_default"_ls, newPowerLevel);
}

int NeoChatRoom::invitePowerLevel() const
{
    return powerLevel("invite"_ls);
}

void NeoChatRoom::setInvitePowerLevel(const int &newPowerLevel)
{
    setPowerLevel("invite"_ls, newPowerLevel);
}

int NeoChatRoom::kickPowerLevel() const
{
    return powerLevel("kick"_ls);
}

void NeoChatRoom::setKickPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("kick"_ls, newPowerLevel);
}

int NeoChatRoom::banPowerLevel() const
{
    return powerLevel("ban"_ls);
}

void NeoChatRoom::setBanPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("ban"_ls, newPowerLevel);
}

int NeoChatRoom::redactPowerLevel() const
{
    return powerLevel("redact"_ls);
}

void NeoChatRoom::setRedactPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("redact"_ls, newPowerLevel);
}

int NeoChatRoom::statePowerLevel() const
{
    return powerLevel("state_default"_ls);
}

void NeoChatRoom::setStatePowerLevel(const int &newPowerLevel)
{
    setPowerLevel("state_default"_ls, newPowerLevel);
}

int NeoChatRoom::defaultEventPowerLevel() const
{
    return powerLevel("events_default"_ls);
}

void NeoChatRoom::setDefaultEventPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("events_default"_ls, newPowerLevel);
}

int NeoChatRoom::powerLevelPowerLevel() const
{
    return powerLevel("m.room.power_levels"_ls, true);
}

void NeoChatRoom::setPowerLevelPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("m.room.power_levels"_ls, newPowerLevel, true);
}

int NeoChatRoom::namePowerLevel() const
{
    return powerLevel("m.room.name"_ls, true);
}

void NeoChatRoom::setNamePowerLevel(const int &newPowerLevel)
{
    setPowerLevel("m.room.name"_ls, newPowerLevel, true);
}

int NeoChatRoom::avatarPowerLevel() const
{
    return powerLevel("m.room.avatar"_ls, true);
}

void NeoChatRoom::setAvatarPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("m.room.avatar"_ls, newPowerLevel, true);
}

int NeoChatRoom::canonicalAliasPowerLevel() const
{
    return powerLevel("m.room.canonical_alias"_ls, true);
}

void NeoChatRoom::setCanonicalAliasPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("m.room.canonical_alias"_ls, newPowerLevel, true);
}

int NeoChatRoom::topicPowerLevel() const
{
    return powerLevel("m.room.topic"_ls, true);
}

void NeoChatRoom::setTopicPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("m.room.topic"_ls, newPowerLevel, true);
}

int NeoChatRoom::encryptionPowerLevel() const
{
    return powerLevel("m.room.encryption"_ls, true);
}

void NeoChatRoom::setEncryptionPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("m.room.encryption"_ls, newPowerLevel, true);
}

int NeoChatRoom::historyVisibilityPowerLevel() const
{
    return powerLevel("m.room.history_visibility"_ls, true);
}

void NeoChatRoom::setHistoryVisibilityPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("m.room.history_visibility"_ls, newPowerLevel, true);
}

int NeoChatRoom::pinnedEventsPowerLevel() const
{
    return powerLevel("m.room.pinned_events"_ls, true);
}

void NeoChatRoom::setPinnedEventsPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("m.room.pinned_events"_ls, newPowerLevel, true);
}

int NeoChatRoom::tombstonePowerLevel() const
{
    return powerLevel("m.room.tombstone"_ls, true);
}

void NeoChatRoom::setTombstonePowerLevel(const int &newPowerLevel)
{
    setPowerLevel("m.room.tombstone"_ls, newPowerLevel, true);
}

int NeoChatRoom::serverAclPowerLevel() const
{
    return powerLevel("m.room.server_acl"_ls, true);
}

void NeoChatRoom::setServerAclPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("m.room.server_acl"_ls, newPowerLevel, true);
}

int NeoChatRoom::spaceChildPowerLevel() const
{
    return powerLevel("m.space.child"_ls, true);
}

void NeoChatRoom::setSpaceChildPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("m.space.child"_ls, newPowerLevel, true);
}

int NeoChatRoom::spaceParentPowerLevel() const
{
    return powerLevel("m.space.parent"_ls, true);
}

void NeoChatRoom::setSpaceParentPowerLevel(const int &newPowerLevel)
{
    setPowerLevel("m.space.parent"_ls, newPowerLevel, true);
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
        co_await qCoro(job, &BaseJob::finished);
        if (job->error() != BaseJob::Success) {
            qWarning() << "Error: \"" << job->error() << "\" while deleting messages. Aborting";
            break;
        }
    }
}

void NeoChatRoom::clearInvitationNotification()
{
    NotificationsManager::instance().clearInvitationNotification(id());
}

bool NeoChatRoom::hasParent() const
{
    return currentState().eventsOfType("m.space.parent"_ls).size() > 0;
}

QList<QString> NeoChatRoom::parentIds() const
{
    auto parentEvents = currentState().eventsOfType("m.space.parent"_ls);
    QList<QString> parentIds;
    for (const auto &parentEvent : parentEvents) {
        if (parentEvent->contentJson().contains("via"_ls) && !parentEvent->contentPart<QJsonArray>("via"_ls).isEmpty()) {
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
    auto parentEvents = currentState().eventsOfType("m.space.parent"_ls);
    for (const auto &parentEvent : parentEvents) {
        if (parentEvent->contentJson().contains("via"_ls) && !parentEvent->contentPart<QJsonArray>("via"_ls).isEmpty()) {
            if (parentEvent->contentPart<bool>("canonical"_ls)) {
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
    if (const auto &parent = currentState().get("m.space.parent"_ls, parentId)) {
        auto content = parent->contentJson();
        content.insert("canonical"_ls, true);
        setState("m.space.parent"_ls, parentId, content);
    } else {
        return;
    }

    // Only one canonical parent can exist so make sure others are set false.
    auto parentEvents = currentState().eventsOfType("m.space.parent"_ls);
    for (const auto &parentEvent : parentEvents) {
        if (parentEvent->contentPart<bool>("canonical"_ls) && parentEvent->stateKey() != parentId) {
            auto content = parentEvent->contentJson();
            content.insert("canonical"_ls, false);
            setState("m.space.parent"_ls, parentEvent->stateKey(), content);
        }
    }
}

bool NeoChatRoom::canModifyParent(const QString &parentId) const
{
    if (!canSendState("m.space.parent"_ls)) {
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
        if (parent->canSendState("m.space.child"_ls)) {
            return true;
        }
        // If the parent has a space child event the user can set as a parent (even
        // if they don't have permission to set space child events in that parent).
        if (parent->currentState().contains("m.space.child"_ls, id())) {
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
        auto parentEvents = currentState().eventsOfType("m.space.parent"_ls);
        for (const auto &parentEvent : parentEvents) {
            if (parentEvent->contentPart<bool>("canonical"_ls)) {
                auto content = parentEvent->contentJson();
                content.insert("canonical"_ls, false);
                setState("m.space.parent"_ls, parentEvent->stateKey(), content);
            }
        }
    }

    setState("m.space.parent"_ls, parentId, QJsonObject{{"canonical"_ls, canonical}, {"via"_ls, QJsonArray{connection()->domain()}}});

    if (setParentChild) {
        if (auto parent = static_cast<NeoChatRoom *>(connection()->room(parentId))) {
            parent->setState("m.space.child"_ls, id(), QJsonObject{{QLatin1String("via"), QJsonArray{connection()->domain()}}});
        }
    }
}

void NeoChatRoom::removeParent(const QString &parentId)
{
    if (!canModifyParent(parentId)) {
        return;
    }
    if (!currentState().contains("m.space.parent"_ls, parentId)) {
        return;
    }
    setState("m.space.parent"_ls, parentId, {});
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
    if (!canSendEvent("m.space.child"_ls)) {
        return;
    }
    setState("m.space.child"_ls,
             childId,
             QJsonObject{{QLatin1String("via"), QJsonArray{connection()->domain()}}, {"suggested"_ls, suggested}, {"order"_ls, order}});

    if (setChildParent) {
        if (auto child = static_cast<NeoChatRoom *>(connection()->room(childId))) {
            if (child->canSendState("m.space.parent"_ls)) {
                child->setState("m.space.parent"_ls, id(), QJsonObject{{"canonical"_ls, canonical}, {"via"_ls, QJsonArray{connection()->domain()}}});

                if (canonical) {
                    // Only one canonical parent can exist so make sure others are set to false.
                    auto parentEvents = child->currentState().eventsOfType("m.space.parent"_ls);
                    for (const auto &parentEvent : parentEvents) {
                        if (parentEvent->contentPart<bool>("canonical"_ls)) {
                            auto content = parentEvent->contentJson();
                            content.insert("canonical"_ls, false);
                            setState("m.space.parent"_ls, parentEvent->stateKey(), content);
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
    if (!canSendEvent("m.space.child"_ls)) {
        return;
    }
    setState("m.space.child"_ls, childId, {});

    if (unsetChildParent) {
        if (auto child = static_cast<NeoChatRoom *>(connection()->room(childId))) {
            if (child->canSendState("m.space.parent"_ls) && child->currentState().contains("m.space.parent"_ls, id())) {
                child->setState("m.space.parent"_ls, id(), {});
            }
        }
    }
}

bool NeoChatRoom::isSuggested(const QString &childId)
{
    if (!currentState().contains("m.space.child"_ls, childId)) {
        return false;
    }
    const auto childEvent = currentState().get("m.space.child"_ls, childId);
    return childEvent->contentPart<bool>("suggested"_ls);
}

void NeoChatRoom::toggleChildSuggested(const QString &childId)
{
    if (!isSpace()) {
        return;
    }
    if (!canSendEvent("m.space.child"_ls)) {
        return;
    }
    if (const auto childEvent = currentState().get("m.space.child"_ls, childId)) {
        auto content = childEvent->contentJson();
        content.insert("suggested"_ls, !childEvent->contentPart<bool>("suggested"_ls));
        setState("m.space.child"_ls, childId, content);
    }
}

void NeoChatRoom::setChildOrder(const QString &childId, const QString &order)
{
    if (!isSpace()) {
        return;
    }
    if (!canSendEvent("m.space.child"_ls)) {
        return;
    }
    if (const auto childEvent = currentState().get("m.space.child"_ls, childId)) {
        auto content = childEvent->contentJson();
        if (!content.contains("via"_ls)) {
            return;
        }
        if (content.value("order"_ls).toString() == order) {
            return;
        }

        content.insert("order"_ls, order);
        setState("m.space.child"_ls, childId, content);
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
    QJsonObject accountData = connection()->accountDataJson("m.push_rules"_ls);

    // For default and mute check for a room rule and remove if found.
    if (state == PushNotificationState::Default || state == PushNotificationState::Mute) {
        QJsonArray roomRuleArray = accountData["global"_ls].toObject()["room"_ls].toArray();
        for (const auto &i : roomRuleArray) {
            QJsonObject roomRule = i.toObject();
            if (roomRule["rule_id"_ls] == id()) {
                connection()->callApi<DeletePushRuleJob>("global"_ls, "room"_ls, id());
            }
        }
    }

    // For default, all and @mentions and keywords check for an override rule and remove if found.
    if (state == PushNotificationState::Default || state == PushNotificationState::All || state == PushNotificationState::MentionKeyword) {
        QJsonArray overrideRuleArray = accountData["global"_ls].toObject()["override"_ls].toArray();
        for (const auto &i : overrideRuleArray) {
            QJsonObject overrideRule = i.toObject();
            if (overrideRule["rule_id"_ls] == id()) {
                connection()->callApi<DeletePushRuleJob>("global"_ls, "override"_ls, id());
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
        const QList<QVariant> actions = {"dont_notify"_ls};
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
        pushCondition.kind = "event_match"_ls;
        pushCondition.key = "room_id"_ls;
        pushCondition.pattern = id();
        const QList<PushCondition> conditions = {pushCondition};

        // Add new override rule and make sure it's enabled
        auto job = connection()->callApi<SetPushRuleJob>("global"_ls, "override"_ls, id(), actions, QString(), QString(), conditions, QString());
        connect(job, &BaseJob::success, this, [this]() {
            auto enableJob = connection()->callApi<SetPushRuleEnabledJob>("global"_ls, "override"_ls, id(), true);
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
        const QList<QVariant> actions = {"dont_notify"_ls};
        // No conditions for a room rule
        const QList<PushCondition> conditions;

        auto setJob = connection()->callApi<SetPushRuleJob>("global"_ls, "room"_ls, id(), actions, QString(), QString(), conditions, QString());
        connect(setJob, &BaseJob::success, this, [this]() {
            auto enableJob = connection()->callApi<SetPushRuleEnabledJob>("global"_ls, "room"_ls, id(), true);
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
        tweaks.insert("set_tweak"_ls, "sound"_ls);
        tweaks.insert("value"_ls, "default"_ls);
        const QList<QVariant> actions = {"notify"_ls, tweaks};
        // No conditions for a room rule
        const QList<PushCondition> conditions;

        // Add new room rule and make sure enabled
        auto setJob = connection()->callApi<SetPushRuleJob>("global"_ls, "room"_ls, id(), actions, QString(), QString(), conditions, QString());
        connect(setJob, &BaseJob::success, this, [this]() {
            auto enableJob = connection()->callApi<SetPushRuleEnabledJob>("global"_ls, "room"_ls, id(), true);
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
    if (type != "m.push_rules"_ls || m_pushNotificationStateUpdating) {
        return;
    }

    QJsonObject accountData = connection()->accountDataJson("m.push_rules"_ls);

    // First look for a room rule with the room id
    QJsonArray roomRuleArray = accountData["global"_ls].toObject()["room"_ls].toArray();
    for (const auto &i : roomRuleArray) {
        QJsonObject roomRule = i.toObject();
        if (roomRule["rule_id"_ls] == id()) {
            if (roomRule["actions"_ls].toArray().size() == 0) {
                m_currentPushNotificationState = PushNotificationState::MentionKeyword;
                Q_EMIT pushNotificationStateChanged(m_currentPushNotificationState);
                return;
            }
            QString notifyAction = roomRule["actions"_ls].toArray()[0].toString();
            if (notifyAction == "notify"_ls) {
                m_currentPushNotificationState = PushNotificationState::All;
                Q_EMIT pushNotificationStateChanged(m_currentPushNotificationState);
                return;
            } else if (notifyAction == "dont_notify"_ls) {
                m_currentPushNotificationState = PushNotificationState::MentionKeyword;
                Q_EMIT pushNotificationStateChanged(m_currentPushNotificationState);
                return;
            }
        }
    }

    // Check for an override rule with the room id
    QJsonArray overrideRuleArray = accountData["global"_ls].toObject()["override"_ls].toArray();
    for (const auto &i : overrideRuleArray) {
        QJsonObject overrideRule = i.toObject();
        if (overrideRule["rule_id"_ls] == id()) {
            if (overrideRule["actions"_ls].toArray().isEmpty()) {
                m_currentPushNotificationState = PushNotificationState::Mute;
                Q_EMIT pushNotificationStateChanged(m_currentPushNotificationState);
                return;
            }
            QString notifyAction = overrideRule["actions"_ls].toArray()[0].toString();
            if (notifyAction == "dont_notify"_ls) {
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
            Q_EMIT showMessage(Positive, i18n("Report sent successfully."));
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
        if (event->hasFileContent()) {
            const auto transferInfo = fileTransferInfo(eventId);
            if (transferInfo.completed()) {
                UrlHelper helper;
                helper.openUrl(transferInfo.localPath);
            } else {
                downloadFile(eventId,
                             QUrl(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + u'/'
                                  + event->id().replace(u':', u'_').replace(u'/', u'_').replace(u'+', u'_') + fileNameToDownload(eventId)));
                connect(this, &Room::fileTransferCompleted, this, [this, eventId](QString id, QUrl localFile, FileSourceInfo fileMetadata) {
                    Q_UNUSED(localFile);
                    Q_UNUSED(fileMetadata);
                    if (id == eventId) {
                        auto transferInfo = fileTransferInfo(eventId);
                        UrlHelper helper;
                        helper.openUrl(transferInfo.localPath);
                    }
                });
            }
        }
    }
}

void NeoChatRoom::copyEventMedia(const QString &eventId)
{
    const auto evtIt = findInTimeline(eventId);
    if (evtIt != messageEvents().rend() && is<RoomMessageEvent>(**evtIt)) {
        const auto event = evtIt->viewAs<RoomMessageEvent>();
        if (event->hasFileContent()) {
            const auto transferInfo = fileTransferInfo(eventId);
            if (transferInfo.completed()) {
                Clipboard clipboard;
                clipboard.setImage(transferInfo.localPath);
            } else {
                downloadFile(eventId,
                             QUrl(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + u'/'
                                  + event->id().replace(u':', u'_').replace(u'/', u'_').replace(u'+', u'_') + fileNameToDownload(eventId)));
                connect(this, &Room::fileTransferCompleted, this, [this, eventId](QString id, QUrl localFile, FileSourceInfo fileMetadata) {
                    Q_UNUSED(localFile);
                    Q_UNUSED(fileMetadata);
                    if (id == eventId) {
                        auto transferInfo = fileTransferInfo(eventId);
                        Clipboard clipboard;
                        clipboard.setImage(transferInfo.localPath);
                    }
                });
            }
        }
    }
}

ChatBarCache *NeoChatRoom::mainCache() const
{
    return m_mainCache;
}

ChatBarCache *NeoChatRoom::editCache() const
{
    return m_editCache;
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
            if (content.contains("m.new_content"_ls)) {
                // The message has been edited so we have to return the id of the original message instead of the replacement
                eventId = content["m.relates_to"_ls].toObject()["event_id"_ls].toString();
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
                if (content.contains("m.new_content"_ls)) {
                    // The message has been edited so we have to return the id of the original message instead of the replacement
                    eventId = content["m.relates_to"_ls].toObject()["event_id"_ls].toString();
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
    return !usesEncryption() && canSendState("m.room.encryption"_ls);
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
    connect(this, &Room::fileTransferFailed, job, &FileTransferPseudoJob::fileTransferFailed);
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

Quotient::User *NeoChatRoom::directChatRemoteUser() const
{
    auto users = connection()->directChatUsers(this);
    if (users.isEmpty()) {
        return nullptr;
    }
    return users[0];
}

void NeoChatRoom::sendLocation(float lat, float lon, const QString &description)
{
    QJsonObject locationContent{
        {"uri"_ls, "geo:%1,%2"_ls.arg(QString::number(lat), QString::number(lon))},
    };

    if (!description.isEmpty()) {
        locationContent["description"_ls] = description;
    }

    QJsonObject content{
        {"body"_ls, i18nc("'Lat' and 'Lon' as in Latitude and Longitude", "Lat: %1, Lon: %2", lat, lon)},
        {"msgtype"_ls, "m.location"_ls},
        {"geo_uri"_ls, "geo:%1,%2"_ls.arg(QString::number(lat), QString::number(lon))},
        {"org.matrix.msc3488.location"_ls, locationContent},
        {"org.matrix.msc3488.asset"_ls,
         QJsonObject{
             {"type"_ls, "m.pin"_ls},
         }},
        {"org.matrix.msc1767.text"_ls, i18nc("'Lat' and 'Lon' as in Latitude and Longitude", "Lat: %1, Lon: %2", lat, lon)},
    };
    postJson("m.room.message"_ls, content);
}

QByteArray NeoChatRoom::roomAcountDataJson(const QString &eventType)
{
    return QJsonDocument(accountData(eventType)->fullJson()).toJson();
}

QUrl NeoChatRoom::avatarForMember(Quotient::User *user) const
{
    const auto &avatar = memberAvatar(user->id());
    if (avatar.url().isEmpty() || avatar.url().scheme() != "mxc"_ls) {
        return {};
    }

    auto localFile = connection()->makeMediaUrl(avatar.url());
    if (!localFile.isValid() || localFile.scheme() != QStringLiteral("mxc")) {
        return QUrl();
    }
    return localFile;
}

const RoomEvent *NeoChatRoom::getReplyForEvent(const RoomEvent &event) const
{
    const QString &replyEventId = event.contentJson()["m.relates_to"_ls].toObject()["m.in_reply_to"_ls].toObject()["event_id"_ls].toString();
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

void NeoChatRoom::loadReply(const QString &eventId, const QString &replyId)
{
    auto job = connection()->callApi<GetOneRoomEventJob>(id(), replyId);
    connect(job, &BaseJob::success, this, [this, job, eventId, replyId] {
        m_extraEvents.push_back(fromJson<event_ptr_tt<RoomEvent>>(job->jsonData()));
        Q_EMIT replyLoaded(eventId, replyId);
    });
}

User *NeoChatRoom::invitingUser() const
{
    return connection()->user(currentState().get<RoomMemberEvent>(connection()->userId())->senderId());
}

void NeoChatRoom::setRoomState(const QString &type, const QString &stateKey, const QByteArray &content)
{
    setState(type, stateKey, QJsonDocument::fromJson(content).object());
}

#include "moc_neochatroom.cpp"
