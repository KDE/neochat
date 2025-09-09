// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <Quotient/events/roomevent.h>
#include <Quotient/room.h>

#include <QCache>
#include <QObject>
#include <QQmlEngine>

#include <QCoroTask>

#include "enums/chatbartype.h"
#include "enums/messagetype.h"
#include "enums/pushrule.h"
#include "events/pollevent.h"
#include "neochatroommember.h"

namespace Quotient
{
class User;
}

class ChatBarCache;

/**
 * @class NeoChatRoom
 *
 * This class is designed to act as a wrapper over Quotient::Room to provide API and
 * functionality not available in Quotient::Room.
 *
 * The functions fall into two main categories:
 *  - Helper functions to make functionality easily accessible in QML.
 *  - Implement functions not yet available in Quotient::Room.
 *
 * @sa Quotient::Room
 */
class NeoChatRoom : public Quotient::Room
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief Convenience function to get the QDateTime of the last event.
     *
     * @sa lastEvent()
     */
    Q_PROPERTY(QDateTime lastActiveTime READ lastActiveTime NOTIFY lastActiveTimeChanged)

    /**
     * @brief Whether a file is being uploaded to the server.
     */
    Q_PROPERTY(bool hasFileUploading READ hasFileUploading WRITE setHasFileUploading NOTIFY hasFileUploadingChanged)

    /**
     * @brief Progress of a file upload as a percentage 0 - 100%.
     *
     * The value will be 0 if no file is uploading.
     *
     * @sa hasFileUploading
     */
    Q_PROPERTY(int fileUploadingProgress READ fileUploadingProgress NOTIFY fileUploadingProgressChanged)

    /**
     * @brief Whether the read marker should be shown.
     */
    Q_PROPERTY(bool readMarkerLoaded READ readMarkerLoaded NOTIFY readMarkerLoadedChanged)

    /**
     * @brief The avatar image to be used for the room, as a mxc:// URL.
     */
    Q_PROPERTY(QUrl avatarMediaUrl READ avatarMediaUrl NOTIFY avatarChanged STORED false)

    /**
     * @brief Get a RoomMember object for the other person in a direct chat.
     */
    Q_PROPERTY(NeochatRoomMember *directChatRemoteMember READ directChatRemoteMember CONSTANT)

    /**
     * @brief The Matrix IDs of this room's parents.
     *
     * Empty if no parent space is set.
     */
    Q_PROPERTY(QList<QString> parentIds READ parentIds NOTIFY parentIdsChanged)

    /**
     * @brief The current canonical parent for the room.
     *
     * Empty if no canonical parent is set. The write method can only be used to
     * set an existing parent as canonical; If you wish to add a new parent and set
     * it as canonical use the addParent method and pass true to the canonical
     * parameter.
     *
     * Setting will fail if the user doesn't have the required privileges (see
     * canModifyParent) or if the given room ID is not a parent room.
     *
     * @sa canModifyParent, addParent
     */
    Q_PROPERTY(QString canonicalParent READ canonicalParent WRITE setCanonicalParent NOTIFY canonicalParentChanged)

    /**
     * @brief If the room is a space.
     */
    Q_PROPERTY(bool isSpace READ isSpace CONSTANT)

    /**
     * @brief The number of notifications in this room's children.
     *
     * Will always return 0 if this is not a space.
     */
    Q_PROPERTY(qsizetype childrenNotificationCount READ childrenNotificationCount NOTIFY childrenNotificationCountChanged)

    /**
     * @brief Whether this room's children have any highlight notifications.
     *
     * Will always return false if this is not a space.
     */
    Q_PROPERTY(bool childrenHaveHighlightNotifications READ childrenHaveHighlightNotifications NOTIFY childrenHaveHighlightNotificationsChanged)

    /**
     * @brief Whether the local user has an invite to the room.
     *
     * False for any other state including if the local user is a member.
     */
    Q_PROPERTY(bool isInvite READ isInvite NOTIFY isInviteChanged)

    /**
     * @brief Whether the local user can send messages in the room.
     */
    Q_PROPERTY(bool readOnly READ readOnly NOTIFY readOnlyChanged)

    /**
     * @brief Get the maximum room version that the server supports.
     *
     * Only returns main integer room versions (i.e. no msc room versions).
     */
    Q_PROPERTY(int maxRoomVersion READ maxRoomVersion NOTIFY maxRoomVersionChanged)

    /**
     * @brief The rule for which messages should generate notifications for the room.
     *
     * @sa PushNotificationState::State
     */
    Q_PROPERTY(PushNotificationState::State pushNotificationState READ pushNotificationState WRITE setPushNotificationState NOTIFY pushNotificationStateChanged)

    /**
     * @brief The current history visibilty setting for the room.
     *
     * Possible values are [invited, joined, shared, world_readable].
     *
     * @sa https://spec.matrix.org/v1.5/client-server-api/#room-history-visibility
     */
    Q_PROPERTY(QString historyVisibility READ historyVisibility WRITE setHistoryVisibility NOTIFY historyVisibilityChanged)

    /**
     * @brief Set the default URL preview state for room members.
     *
     * Assumed false if the org.matrix.room.preview_urls state message has never been
     * set. Can only be set if the calling user has a high enough power level.
     */
    Q_PROPERTY(bool defaultUrlPreviewState READ defaultUrlPreviewState WRITE setDefaultUrlPreviewState NOTIFY defaultUrlPreviewStateChanged)

    /**
     * @brief Enable URL previews for the local user.
     */
    Q_PROPERTY(bool urlPreviewEnabled READ urlPreviewEnabled WRITE setUrlPreviewEnabled NOTIFY urlPreviewEnabledChanged)

    /**
     * @brief Whether the local user can encrypt the room.
     *
     * A local user can encrypt a room if they have permission to send the m.room.encryption
     * state event.
     *
     * @sa https://spec.matrix.org/v1.5/client-server-api/#mroomencryption
     */
    Q_PROPERTY(bool canEncryptRoom READ canEncryptRoom NOTIFY canEncryptRoomChanged)

    /**
     * @brief The cache for the main chat bar in the room.
     */
    Q_PROPERTY(ChatBarCache *mainCache READ mainCache CONSTANT)

    /**
     * @brief The cache for the edit chat bar in the room.
     */
    Q_PROPERTY(ChatBarCache *editCache READ editCache CONSTANT)

    /**
     * @brief The cache for the thread chat bar in the room.
     */
    Q_PROPERTY(ChatBarCache *threadCache READ threadCache CONSTANT)

    /**
     * @brief When the current user was invited to the room.
     */
    Q_PROPERTY(QDateTime inviteTimestamp READ inviteTimestamp NOTIFY baseStateLoaded)

    /**
     * @brief When the current user was invited to the room.
     */
    Q_PROPERTY(QString invitingUserId READ invitingUserId NOTIFY baseStateLoaded)

    /**
     * @brief The most recently pinned message in the room.
     */
    Q_PROPERTY(QString pinnedMessage READ pinnedMessage NOTIFY pinnedMessageChanged)

public:
    explicit NeoChatRoom(Quotient::Connection *connection, QString roomId, Quotient::JoinState joinState = {});

    bool visible() const;
    void setVisible(bool visible);

    [[nodiscard]] QDateTime lastActiveTime() const;

    /**
     * @brief Get the last interesting event.
     *
     * This function respects the user's state event setting and discards
     * other not interesting events.
     *
     * @warning This function can return an empty pointer if the room does not have
     *          any RoomMessageEvents loaded.
     */
    [[nodiscard]] const Quotient::RoomEvent *lastEvent(std::function<bool(const Quotient::RoomEvent *)> filter = {}) const;

    /**
     * @brief Convenient way to check if the event looks like it has spoilers.
     *
     * This does a basic check to see if the message contains a data-mx-spoiler
     * attribute within the text which makes it likely that the message has a spoiler
     * section. However this is not 100% reliable as during parsing it may be
     * removed if used within an illegal tag or on a tag for which data-mx-spoiler
     * is not a valid attribute.
     *
     * @sa lastEvent()
     */
    [[nodiscard]] bool isEventSpoiler(const Quotient::RoomEvent *e) const;

    /**
     * @brief Return the notification count for the room accounting for tags and notification state.
     *
     * The following rules are observed:
     *  - Rooms tagged as low priority or mentions and keywords notification state
     *    only return the number of highlights.
     *  - Muted rooms always return 0.
     */
    int contextAwareNotificationCount() const;

    [[nodiscard]] bool hasFileUploading() const;
    void setHasFileUploading(bool value);

    [[nodiscard]] int fileUploadingProgress() const;
    void setFileUploadingProgress(int value);

    /**
     * @brief Download a file for the given event to a local file location.
     */
    Q_INVOKABLE void download(const QString &eventId, const QUrl &localFilename = {});

    /**
     * @brief Download a file for the given event as a temporary file.
     */
    Q_INVOKABLE bool downloadTempFile(const QString &eventId);

    /**
     * @brief Check if the given event is highlighted.
     *
     * An event is highlighted if it contains the local user's id but was not sent by the
     * local user.
     */
    bool isEventHighlighted(const Quotient::RoomEvent *e) const;

    /**
     * @brief Convenience function to find out if the room contains the given user.
     *
     * A room contains the user if the user can be found and their JoinState is
     * not JoinState::Leave.
     */
    Q_INVOKABLE [[nodiscard]] bool containsUser(const QString &userID) const;

    /**
     * @brief True if the given user ID is banned from the room.
     */
    Q_INVOKABLE [[nodiscard]] bool isUserBanned(const QString &user) const;

    /**
     * @brief True if the local user can send the given event type.
     */
    Q_INVOKABLE [[nodiscard]] bool canSendEvent(const QString &eventType) const;

    /**
     * @brief True if the local user can send the given state event type.
     */
    Q_INVOKABLE [[nodiscard]] bool canSendState(const QString &eventType) const;

    /**
     * @brief Send a report to the server for an event.
     *
     * @param eventId the ID of the event being reported.
     * @param reason the reason given for reporting the event.
     */
    Q_INVOKABLE void reportEvent(const QString &eventId, const QString &reason);

    Q_INVOKABLE QByteArray getEventJsonSource(const QString &eventId);

    /**
     * @brief Open the media for the given event in an appropriate external app.
     *
     * Will do nothing if the event has no media.
     */
    Q_INVOKABLE void openEventMediaExternally(const QString &eventId);

    /**
     * @brief Copy the media for the given event to the clipboard.
     *
     * Will do nothing if the event has no media.
     */
    Q_INVOKABLE void copyEventMedia(const QString &eventId);

    [[nodiscard]] bool readMarkerLoaded() const;

    [[nodiscard]] QUrl avatarMediaUrl() const;

    NeochatRoomMember *directChatRemoteMember();

    /**
     * @brief Whether this room has one or more parent spaces set.
     */
    Q_INVOKABLE bool hasParent() const;

    QList<QString> parentIds() const;

    /**
     * @brief Get a list of parent space objects for this room.
     *
     * Will only return retrun spaces that are know, i.e. the user has joined and
     * a valid NeoChatRoom is available.
     *
     * @param multiLevel whether the function should recursively gather all levels
     *        of parents
     */
    Q_INVOKABLE QList<NeoChatRoom *> parentObjects(bool multiLevel = false) const;

    QString canonicalParent() const;
    void setCanonicalParent(const QString &parentId);

    /**
     * @brief Whether the local user has permission to set the given space as a parent.
     *
     * @note This follows the rules determined in the Matrix spec
     *       https://spec.matrix.org/v1.7/client-server-api/#mspaceparent-relationships
     */
    Q_INVOKABLE bool canModifyParent(const QString &parentId) const;

    /**
     * @brief Add the given room as a parent.
     *
     * Will fail if the user doesn't have the required privileges (see
     * canModifyParent()).
     *
     * @sa canModifyParent()
     */
    Q_INVOKABLE void addParent(const QString &parentId, bool canonical = false, bool setParentChild = false);

    /**
     * @brief Remove the given room as a parent.
     *
     * Will fail if the user doesn't have the required privileges (see
     * canModifyParent()).
     *
     * @sa canModifyParent()
     */
    Q_INVOKABLE void removeParent(const QString &parentId);

    [[nodiscard]] bool isSpace() const;

    qsizetype childrenNotificationCount();

    bool childrenHaveHighlightNotifications() const;

    /**
     * @brief Add the given room as a child.
     *
     * Will fail if the user doesn't have the required privileges or this room is
     * not a space.
     */
    Q_INVOKABLE void addChild(const QString &childId, bool setChildParent = false, bool canonical = false, bool suggested = false, const QString &order = {});

    /**
     * @brief Remove the given room as a child.
     *
     * Will fail if the user doesn't have the required privileges or this room is
     * not a space.
     */
    Q_INVOKABLE void removeChild(const QString &childId, bool unsetChildParent = false);

    /**
     * @brief Whether the given child is a suggested room in the space.
     */
    Q_INVOKABLE bool isSuggested(const QString &childId);

    /**
     * @brief Toggle whether the given child is a suggested room in the space.
     *
     * Will fail if the user doesn't have the required privileges, this room is
     * not a space or the given room is not a child of this space.
     */
    Q_INVOKABLE void toggleChildSuggested(const QString &childId);

    void setChildOrder(const QString &childId, const QString &order = {});

    bool isInvite() const;

    bool readOnly() const;

    int maxRoomVersion() const;

    /**
     * @brief Map an alias to the room and publish.
     *
     * The alias is first mapped to the room and then published as an
     * alternate alias. Publishing the alias will fail if the user does not have
     * permission to send m.room.canonical_alias event messages.
     *
     * @note This is different to Quotient::Room::setLocalAliases() as that can only
     * get the room to publish an alias that is already mapped.
     *
     * @property alias QString in the form #new_alias:server.org
     *
     * @sa Quotient::Room::setLocalAliases()
     */
    Q_INVOKABLE void mapAlias(const QString &alias);

    /**
     * @brief Unmap an alias from the room.
     *
     * An unmapped alias is also removed as either the canonical alias or an alternate
     * alias.
     *
     * @note This is different to Quotient::Room::setLocalAliases() as that can only
     * get the room to un-publish an alias, while the mapping still exists.
     *
     * @property alias QString in the form #mapped_alias:server.org
     *
     * @sa Quotient::Room::setLocalAliases()
     */
    Q_INVOKABLE void unmapAlias(const QString &alias);

    /**
     * @brief Set the canonical alias of the room to an available mapped alias.
     *
     * If the new alias was already published as an alternate alias it will be removed
     * from that list.
     *
     * @note This is an overload of the function Quotient::Room::setCanonicalAlias().
     * This is to provide the functionality to remove the new canonical alias as a
     * published alt alias when set.
     *
     * @property newAlias QString in the form #new_alias:server.org
     *
     * @sa Quotient::Room::setCanonicalAlias()
     * */
    Q_INVOKABLE void setCanonicalAlias(const QString &newAlias);

    Q_INVOKABLE void setRoomState(const QString &type, const QString &stateKey, const QByteArray &content);

    PushNotificationState::State pushNotificationState() const;
    void setPushNotificationState(PushNotificationState::State state);

    [[nodiscard]] QString historyVisibility() const;
    void setHistoryVisibility(const QString &historyVisibilityRule);

    [[nodiscard]] bool defaultUrlPreviewState() const;
    void setDefaultUrlPreviewState(const bool &defaultUrlPreviewState);

    [[nodiscard]] bool urlPreviewEnabled() const;
    void setUrlPreviewEnabled(const bool &urlPreviewEnabled);

    bool canEncryptRoom() const;

    Q_INVOKABLE void setUserPowerLevel(const QString &userID, const int &powerLevel);

    ChatBarCache *mainCache() const;

    ChatBarCache *editCache() const;

    ChatBarCache *threadCache() const;

    ChatBarCache *cacheForType(ChatBarType::Type type) const;

    /**
     * @brief Reply to the last message sent in the timeline.
     *
     * @note This checks a maximum of the previous 35 message for performance reasons.
     */
    Q_INVOKABLE void replyLastMessage();

    /**
     * @brief Edit the last message sent by the local user.
     *
     * @note This checks a maximum of the previous 35 message for performance reasons.
     */
    Q_INVOKABLE void editLastMessage();

    Q_INVOKABLE void postPoll(PollKind::Kind kind, const QString &question, const QList<QString> &answers);

    /**
     * @brief Get the full Json data for a given room account data event.
     */
    Q_INVOKABLE QByteArray roomAcountDataJson(const QString &eventType);

    /**
     * @brief Loads the event with the given id from the server and saves it locally.
     *
     * Intended to retrieve events that are needed, e.g. replied to events that are
     * not currently in the timeline.
     *
     * If the event is already in the timeline nothing will happen.
     */
    void downloadEventFromServer(const QString &eventId);

    /**
     * @brief Returns the event with the given ID if available.
     *
     * This function will check both the timeline and extra events and return a
     * non-nullptr value if it is found in either.
     *
     * The result will be nullptr if not found so needs to be managed.
     */
    std::pair<const Quotient::RoomEvent *, bool> getEvent(const QString &eventId) const;

    /**
     * @brief Returns the event that is being replied to. This includes events that were manually loaded using NeoChatRoom::loadReply.
     */
    const Quotient::RoomEvent *getReplyForEvent(const Quotient::RoomEvent &event) const;

    /**
     * If we're invited to this room, the user that invited us. Undefined in other cases.
     */
    QString invitingUserId() const;

    /**
     * If we're invited to this room, the timestamp when we were invited in. Undefined in other cases.
     */
    QDateTime inviteTimestamp() const;

    /**
     * @brief Return the cached file transfer information for the event.
     *
     * If we downloaded the file previously, return a struct with Completed status
     * and the local file path stored in KSharedConfig
     */
    Quotient::FileTransferInfo cachedFileTransferInfo(const QString &eventId) const;

    /**
     * @brief Return the cached file transfer information for the event.
     *
     * If we downloaded the file previously, return a struct with Completed status
     * and the local file path stored in KSharedConfig
     */
    Quotient::FileTransferInfo cachedFileTransferInfo(const Quotient::RoomEvent *event) const;

    /**
     * @brief Return a NeochatRoomMember object for the given user ID.
     *
     * @warning Because we can't guarantee that a member state event is downloaded
     *          before a message they sent arrives this will create the object unconditionally
     *          assuming that the state event will turn up later. It is therefor the
     *          responsibility of the caller to ensure that they only ask for objects
     *          for real senders.
     */
    Q_INVOKABLE NeochatRoomMember *qmlSafeMember(const QString &memberId);

    /**
     * @brief Pin a message in the room.
     * @param eventId The id of the event to pin.
     */
    Q_INVOKABLE void pinEvent(const QString &eventId);

    /**
     * @brief Unpin a message in the room.
     * @param eventId The id of the event to unpin.
     */
    Q_INVOKABLE void unpinEvent(const QString &eventId);

    /**
     * @return True if @p eventId is pinned in the room.
     */
    Q_INVOKABLE bool isEventPinned(const QString &eventId) const;

    /**
     * @return True if the given @p eventId is threaded.
     */
    Q_INVOKABLE bool eventIsThreaded(const QString &eventId) const;

    /**
     * @return Returns the thread root ID for @p eventId as a string. The string
     *         is empty if the event is not part of a thread.
     */
    Q_INVOKABLE QString rootIdForThread(const QString &eventId) const;

    static void setHiddenFilter(std::function<bool(const Quotient::RoomEvent *)> hiddenFilter);

    /**
     * @brief Whether this room has a room version where the creator is treated as having an ultimate power level
     *
     * For unusual room versions, this information might be wrong.
     */
    Q_INVOKABLE bool roomCreatorHasUltimatePowerLevel() const;

    /**
     * @brief Whether this user is considered a creator of this room. Only applies to post-v12 rooms.
     */
    bool isCreator(const QString &userId) const;

    /**
     * @return The most recent pinned message in the room.
     */
    QString pinnedMessage() const;

    /**
     * @brief Send a report about this room.
     */
    Q_INVOKABLE void report(const QString &reason);

private:
    bool m_visible = false;

    QSet<const Quotient::RoomEvent *> highlights;

    bool m_hasFileUploading = false;
    int m_fileUploadingProgress = 0;

    PushNotificationState::State m_currentPushNotificationState = PushNotificationState::Unknown;
    bool m_pushNotificationStateUpdating = false;

    void checkForHighlights(const Quotient::TimelineItem &ti);

    void onAddNewTimelineEvents(timeline_iter_t from) override;
    void onAddHistoricalTimelineEvents(rev_iter_t from) override;
    void onRedaction(const Quotient::RoomEvent &prevEvent, const Quotient::RoomEvent &after) override;

    QCoro::Task<void> doDeleteMessagesByUser(const QString &user, QString reason);
    QCoro::Task<void> doUploadFile(QUrl url, QString body = QString(), std::optional<Quotient::EventRelation> relatesTo = std::nullopt);

    std::unique_ptr<Quotient::RoomEvent> m_cachedEvent;

    ChatBarCache *m_mainCache;
    ChatBarCache *m_editCache;
    ChatBarCache *m_threadCache;

    std::vector<Quotient::event_ptr_tt<Quotient::RoomEvent>> m_extraEvents;
    void cleanupExtraEventRange(Quotient::RoomEventsRange events);
    void cleanupExtraEvent(const QString &eventId);

    std::unordered_map<QString, std::unique_ptr<NeochatRoomMember>> m_memberObjects;
    static std::function<bool(const Quotient::RoomEvent *)> m_hiddenFilter;
    QString m_pinnedMessage;
    void loadPinnedMessage();

private Q_SLOTS:
    void updatePushNotificationState(QString type);

    void cacheLastEvent();

Q_SIGNALS:
    void cachedInputChanged();
    void busyChanged();
    void hasFileUploadingChanged();
    void fileUploadingProgressChanged();
    void backgroundChanged();
    void readMarkerLoadedChanged();
    void parentIdsChanged();
    void canonicalParentChanged();
    void lastActiveTimeChanged();
    void childrenNotificationCountChanged();
    void childrenHaveHighlightNotificationsChanged();
    void isInviteChanged();
    void readOnlyChanged();
    void displayNameChanged();
    void pushNotificationStateChanged(PushNotificationState::State state);
    void canEncryptRoomChanged();
    void historyVisibilityChanged();
    void defaultUrlPreviewStateChanged();
    void urlPreviewEnabledChanged();
    void maxRoomVersionChanged();
    void extraEventLoaded(const QString &eventId);
    void extraEventNotFound(const QString &eventId);
    void inviteTimestampChanged();
    void pinnedMessageChanged();

    /**
     * @brief Request a message be shown to the user of the given type.
     */
    void showMessage(MessageType::Type messageType, const QString &message);

public Q_SLOTS:
    /**
     * @brief Upload a file to the matrix server and post the file to the room.
     *
     * @param url the location of the file to be uploaded.
     * @param body the caption that is to be given to the file.
     */
    void uploadFile(const QUrl &url, const QString &body = QString(), std::optional<Quotient::EventRelation> relatesTo = std::nullopt);

    /**
     * @brief Accept an invitation for the local user to join the room.
     */
    void acceptInvitation();

    /**
     * @brief Leave and forget the room for the local user.
     *
     * @note This means that not only will the user no longer receive events in
     *       the room but the will forget any history up to this point.
     *
     * @sa https://spec.matrix.org/latest/client-server-api/#leaving-rooms
     */
    void forget();

    /**
     * @brief Set the typing notification state on the room for the local user.
     */
    void sendTypingNotification(bool isTyping);

    /**
     * @brief Set the room avatar.
     */
    void changeAvatar(const QUrl &localFile);

    /**
     * @brief Toggle the reaction state of the given reaction for the local user.
     */
    void toggleReaction(const QString &eventId, const QString &reaction);

    /**
     * @brief Delete recent messages by the given user.
     *
     * This will delete all messages by that user in this room that are currently loaded.
     */
    void deleteMessagesByUser(const QString &user, const QString &reason);

    /**
     *  @brief Sends a location to a room
     *  The event is sent in the migration format as specified in MSC3488
     * @param lat latitude
     * @param lon longitude
     * @param description description for the location
     */
    void sendLocation(float lat, float lon, const QString &description);
};
