// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <room.h>

#include <QCache>
#include <QObject>
#include <QTextCursor>

#include <qcorotask.h>

#include "neochatuser.h"
#include "pollhandler.h"

class PushNotificationState : public QObject
{
    Q_OBJECT

public:
    enum State {
        Unknown,
        Default,
        Mute,
        MentionKeyword,
        All,
    };
    Q_ENUM(State);
};

struct Mention {
    QTextCursor cursor;
    QString text;
    int start = 0;
    int position = 0;
    QString id;
};

class NeoChatRoom : public Quotient::Room
{
    Q_OBJECT
    Q_PROPERTY(QVariantList usersTyping READ getUsersTyping NOTIFY typingChanged)
    Q_PROPERTY(bool hasFileUploading READ hasFileUploading WRITE setHasFileUploading NOTIFY hasFileUploadingChanged)
    Q_PROPERTY(int fileUploadingProgress READ fileUploadingProgress NOTIFY fileUploadingProgressChanged)
    Q_PROPERTY(QString avatarMediaId READ avatarMediaId NOTIFY avatarChanged STORED false)
    Q_PROPERTY(bool readMarkerLoaded READ readMarkerLoaded NOTIFY readMarkerLoadedChanged)
    Q_PROPERTY(QDateTime lastActiveTime READ lastActiveTime NOTIFY lastActiveTimeChanged)
    Q_PROPERTY(bool isSpace READ isSpace CONSTANT)
    Q_PROPERTY(bool isInvite READ isInvite NOTIFY isInviteChanged)
    Q_PROPERTY(QString joinRule READ joinRule WRITE setJoinRule NOTIFY joinRuleChanged)
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

    // Properties for the various permission levels for the room
    Q_PROPERTY(int defaultUserPowerLevel READ defaultUserPowerLevel WRITE setDefaultUserPowerLevel NOTIFY defaultUserPowerLevelChanged)
    Q_PROPERTY(int invitePowerLevel READ invitePowerLevel WRITE setInvitePowerLevel NOTIFY invitePowerLevelChanged)
    Q_PROPERTY(int kickPowerLevel READ kickPowerLevel WRITE setKickPowerLevel NOTIFY kickPowerLevelChanged)
    Q_PROPERTY(int banPowerLevel READ banPowerLevel WRITE setBanPowerLevel NOTIFY banPowerLevelChanged)
    Q_PROPERTY(int redactPowerLevel READ redactPowerLevel WRITE setRedactPowerLevel NOTIFY redactPowerLevelChanged)
    Q_PROPERTY(int statePowerLevel READ statePowerLevel WRITE setStatePowerLevel NOTIFY statePowerLevelChanged)
    Q_PROPERTY(int defaultEventPowerLevel READ defaultEventPowerLevel WRITE setDefaultEventPowerLevel NOTIFY defaultEventPowerLevelChanged)
    Q_PROPERTY(int powerLevelPowerLevel READ powerLevelPowerLevel WRITE setPowerLevelPowerLevel NOTIFY powerLevelPowerLevelChanged)
    Q_PROPERTY(int namePowerLevel READ namePowerLevel WRITE setNamePowerLevel NOTIFY namePowerLevelChanged)
    Q_PROPERTY(int avatarPowerLevel READ avatarPowerLevel WRITE setAvatarPowerLevel NOTIFY avatarPowerLevelChanged)
    Q_PROPERTY(int canonicalAliasPowerLevel READ canonicalAliasPowerLevel WRITE setCanonicalAliasPowerLevel NOTIFY canonicalAliasPowerLevelChanged)
    Q_PROPERTY(int topicPowerLevel READ topicPowerLevel WRITE setTopicPowerLevel NOTIFY topicPowerLevelChanged)
    Q_PROPERTY(int encryptionPowerLevel READ encryptionPowerLevel WRITE setEncryptionPowerLevel NOTIFY encryptionPowerLevelChanged)
    Q_PROPERTY(int historyVisibilityPowerLevel READ historyVisibilityPowerLevel WRITE setHistoryVisibilityPowerLevel NOTIFY historyVisibilityPowerLevelChanged)
    Q_PROPERTY(int pinnedEventsPowerLevel READ pinnedEventsPowerLevel WRITE setPinnedEventsPowerLevel NOTIFY pinnedEventsPowerLevelChanged)
    Q_PROPERTY(int tombstonePowerLevel READ tombstonePowerLevel WRITE setTombstonePowerLevel NOTIFY tombstonePowerLevelChanged)
    Q_PROPERTY(int serverAclPowerLevel READ serverAclPowerLevel WRITE setServerAclPowerLevel NOTIFY serverAclPowerLevelChanged)
    Q_PROPERTY(int spaceChildPowerLevel READ spaceChildPowerLevel WRITE setSpaceChildPowerLevel NOTIFY spaceChildPowerLevelChanged)
    Q_PROPERTY(int spaceParentPowerLevel READ spaceParentPowerLevel WRITE setSpaceParentPowerLevel NOTIFY spaceParentPowerLevelChanged)

    Q_PROPERTY(QString htmlSafeDisplayName READ htmlSafeDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(PushNotificationState::State pushNotificationState READ pushNotificationState WRITE setPushNotificationState NOTIFY pushNotificationStateChanged)

    // Due to problems with QTextDocument, unlike the other properties here, chatBoxText is *not* used to store the text when switching rooms
    Q_PROPERTY(QString chatBoxText READ chatBoxText WRITE setChatBoxText NOTIFY chatBoxTextChanged)

    /**
     * @brief The text for any message currently being edited in the room.
     */
    Q_PROPERTY(QString editText READ editText WRITE setEditText NOTIFY editTextChanged)
    Q_PROPERTY(QString chatBoxReplyId READ chatBoxReplyId WRITE setChatBoxReplyId NOTIFY chatBoxReplyIdChanged)
    Q_PROPERTY(QString chatBoxEditId READ chatBoxEditId WRITE setChatBoxEditId NOTIFY chatBoxEditIdChanged)
    Q_PROPERTY(NeoChatUser *chatBoxReplyUser READ chatBoxReplyUser NOTIFY chatBoxReplyIdChanged)
    Q_PROPERTY(QString chatBoxReplyMessage READ chatBoxReplyMessage NOTIFY chatBoxReplyIdChanged)
    Q_PROPERTY(NeoChatUser *chatBoxEditUser READ chatBoxEditUser NOTIFY chatBoxEditIdChanged)
    Q_PROPERTY(QString chatBoxEditMessage READ chatBoxEditMessage NOTIFY chatBoxEditIdChanged)
    Q_PROPERTY(QString chatBoxAttachmentPath READ chatBoxAttachmentPath WRITE setChatBoxAttachmentPath NOTIFY chatBoxAttachmentPathChanged)
    Q_PROPERTY(bool canEncryptRoom READ canEncryptRoom NOTIFY canEncryptRoomChanged)

    /**
     * @brief Get the maximum room version that the server supports.
     *
     * Only returns main integer room versions (i.e. no msc room versions).
     */
    Q_PROPERTY(int maxRoomVersion READ maxRoomVersion NOTIFY maxRoomVersionChanged)
    Q_PROPERTY(NeoChatUser *directChatRemoteUser READ directChatRemoteUser CONSTANT)

public:
    enum MessageType {
        Positive,
        Info,
        Error,
    };
    Q_ENUM(MessageType);

    explicit NeoChatRoom(Quotient::Connection *connection, QString roomId, Quotient::JoinState joinState = {});

    [[nodiscard]] QVariantList getUsersTyping() const;

    /// Get the interesting last event.
    ///
    /// This function respect the showLeaveJoinEvent setting and discard
    /// other not interesting events. This function can return an empty pointer
    /// when the room is empty of RoomMessageEvent.
    [[nodiscard]] const Quotient::RoomEvent *lastEvent() const;

    /// Convenient way to get the last event but in a string format.
    ///
    /// \see lastEvent
    /// \see lastEventIsSpoiler
    [[nodiscard]] QString lastEventToString(Qt::TextFormat format = Qt::PlainText, bool stripNewlines = false) const;

    /// Convenient way to check if the last event looks like it has spoilers.
    ///
    /// \see lastEvent
    /// \see lastEventToString
    [[nodiscard]] bool lastEventIsSpoiler() const;

    /// Convenient way to get the QDateTime of the last event.
    ///
    /// \see lastEvent
    [[nodiscard]] QDateTime lastActiveTime();

    [[nodiscard]] bool isSpace();

    bool isEventHighlighted(const Quotient::RoomEvent *e) const;

    [[nodiscard]] QString joinRule() const;
    void setJoinRule(const QString &joinRule);

    [[nodiscard]] QString historyVisibility() const;
    void setHistoryVisibility(const QString &historyVisibilityRule);

    [[nodiscard]] bool defaultUrlPreviewState() const;
    void setDefaultUrlPreviewState(const bool &defaultUrlPreviewState);

    [[nodiscard]] bool urlPreviewEnabled() const;
    void setUrlPreviewEnabled(const bool &urlPreviewEnabled);

    /**
     * @brief Get the power level for the given user ID in the room.
     *
     * Returns the default value for a user in the room if they have no escalated
     * privileges or if they are not a member so membership should be known before using.
     */
    Q_INVOKABLE [[nodiscard]] int getUserPowerLevel(const QString &userId) const;

    Q_INVOKABLE void setUserPowerLevel(const QString &userID, const int &powerLevel);

    [[nodiscard]] int powerLevel(const QString &eventName, const bool &isStateEvent = false) const;
    void setPowerLevel(const QString &eventName, const int &newPowerLevel, const bool &isStateEvent = false);

    [[nodiscard]] int defaultUserPowerLevel() const;
    void setDefaultUserPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int invitePowerLevel() const;
    void setInvitePowerLevel(const int &newPowerLevel);

    [[nodiscard]] int kickPowerLevel() const;
    void setKickPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int banPowerLevel() const;
    void setBanPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int redactPowerLevel() const;
    void setRedactPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int statePowerLevel() const;
    void setStatePowerLevel(const int &newPowerLevel);

    [[nodiscard]] int defaultEventPowerLevel() const;
    void setDefaultEventPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int powerLevelPowerLevel() const;
    void setPowerLevelPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int namePowerLevel() const;
    void setNamePowerLevel(const int &newPowerLevel);

    [[nodiscard]] int avatarPowerLevel() const;
    void setAvatarPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int canonicalAliasPowerLevel() const;
    void setCanonicalAliasPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int topicPowerLevel() const;
    void setTopicPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int encryptionPowerLevel() const;
    void setEncryptionPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int historyVisibilityPowerLevel() const;
    void setHistoryVisibilityPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int pinnedEventsPowerLevel() const;
    void setPinnedEventsPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int tombstonePowerLevel() const;
    void setTombstonePowerLevel(const int &newPowerLevel);

    [[nodiscard]] int serverAclPowerLevel() const;
    void setServerAclPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int spaceChildPowerLevel() const;
    void setSpaceChildPowerLevel(const int &newPowerLevel);

    [[nodiscard]] int spaceParentPowerLevel() const;
    void setSpaceParentPowerLevel(const int &newPowerLevel);

    [[nodiscard]] bool hasFileUploading() const;
    void setHasFileUploading(bool value);

    [[nodiscard]] int fileUploadingProgress() const;
    void setFileUploadingProgress(int value);

    [[nodiscard]] bool readMarkerLoaded() const;

    Q_INVOKABLE [[nodiscard]] QVariantList getUsers(const QString &keyword, int limit = -1) const;
    Q_INVOKABLE [[nodiscard]] QVariantMap getUser(const QString &userID) const;

    [[nodiscard]] QString avatarMediaId() const;

    [[nodiscard]] QString eventToString(const Quotient::RoomEvent &evt, Qt::TextFormat format = Qt::PlainText, bool stripNewlines = false) const;
    [[nodiscard]] QString eventToGenericString(const Quotient::RoomEvent &evt) const;

    Q_INVOKABLE [[nodiscard]] bool containsUser(const QString &userID) const;
    Q_INVOKABLE [[nodiscard]] bool isUserBanned(const QString &user) const;

    Q_INVOKABLE [[nodiscard]] bool canSendEvent(const QString &eventType) const;
    Q_INVOKABLE [[nodiscard]] bool canSendState(const QString &eventType) const;

    bool isInvite() const;

    QString htmlSafeDisplayName() const;
    Q_INVOKABLE void clearInvitationNotification();
    Q_INVOKABLE void reportEvent(const QString &eventId, const QString &reason);

    PushNotificationState::State pushNotificationState() const;
    void setPushNotificationState(PushNotificationState::State state);

    Q_INVOKABLE void download(const QString &eventId, const QUrl &localFilename = {});

    QString chatBoxText() const;
    void setChatBoxText(const QString &text);

    QString editText() const;
    void setEditText(const QString &text);

    QString chatBoxReplyId() const;
    void setChatBoxReplyId(const QString &replyId);

    NeoChatUser *chatBoxReplyUser() const;
    QString chatBoxReplyMessage() const;

    QString chatBoxEditId() const;
    void setChatBoxEditId(const QString &editId);

    NeoChatUser *chatBoxEditUser() const;
    QString chatBoxEditMessage() const;

    QString chatBoxAttachmentPath() const;
    void setChatBoxAttachmentPath(const QString &attachmentPath);

    QVector<Mention> *mentions();

    /**
     * @brief Vector of mentions in the current edit text.
     */
    QVector<Mention> *editMentions();

    QString savedText() const;
    void setSavedText(const QString &savedText);

    bool canEncryptRoom() const;

    Q_INVOKABLE bool downloadTempFile(const QString &eventId);

    /*
     * Map an alias to the room
     *
     * Note: this is different to setLocalAliases as that can only
     * get the room to publish and alias that is already mapped.
     */
    Q_INVOKABLE void mapAlias(const QString &alias);
    Q_INVOKABLE void unmapAlias(const QString &alias);
    Q_INVOKABLE void setCanonicalAlias(const QString &newAlias);

#ifdef QUOTIENT_07
    Q_INVOKABLE PollHandler *poll(const QString &eventId);
#endif

#ifndef QUOTIENT_07
    Q_INVOKABLE QString htmlSafeMemberName(const QString &userId) const
    {
        return safeMemberName(userId).toHtmlEscaped();
    }
#endif

    int maxRoomVersion() const;
    NeoChatUser *directChatRemoteUser() const;

private:
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
    QCoro::Task<void> doUploadFile(QUrl url, QString body = QString());

    QString m_chatBoxText;
    QString m_editText;
    QString m_chatBoxReplyId;
    QString m_chatBoxEditId;
    QString m_chatBoxAttachmentPath;
    QVector<Mention> m_mentions;
    QVector<Mention> m_editMentions;
    QString m_savedText;
#ifdef QUOTIENT_07
    QCache<QString, PollHandler> m_polls;
#endif

private Q_SLOTS:
    void countChanged();
    void updatePushNotificationState(QString type);

Q_SIGNALS:
    void cachedInputChanged();
    void busyChanged();
    void hasFileUploadingChanged();
    void fileUploadingProgressChanged();
    void backgroundChanged();
    void readMarkerLoadedChanged();
    void lastActiveTimeChanged();
    void isInviteChanged();
    void displayNameChanged();
    void pushNotificationStateChanged(PushNotificationState::State state);
    void showMessage(MessageType messageType, const QString &message);
    void chatBoxTextChanged();
    void editTextChanged();
    void chatBoxReplyIdChanged();
    void chatBoxEditIdChanged();
    void chatBoxAttachmentPathChanged();
    void canEncryptRoomChanged();
    void joinRuleChanged();
    void historyVisibilityChanged();
    void defaultUrlPreviewStateChanged();
    void urlPreviewEnabledChanged();
    void maxRoomVersionChanged();
    void defaultUserPowerLevelChanged();
    void invitePowerLevelChanged();
    void kickPowerLevelChanged();
    void banPowerLevelChanged();
    void redactPowerLevelChanged();
    void statePowerLevelChanged();
    void defaultEventPowerLevelChanged();
    void powerLevelPowerLevelChanged();
    void namePowerLevelChanged();
    void avatarPowerLevelChanged();
    void canonicalAliasPowerLevelChanged();
    void topicPowerLevelChanged();
    void encryptionPowerLevelChanged();
    void historyVisibilityPowerLevelChanged();
    void pinnedEventsPowerLevelChanged();
    void tombstonePowerLevelChanged();
    void serverAclPowerLevelChanged();
    void spaceChildPowerLevelChanged();
    void spaceParentPowerLevelChanged();

public Q_SLOTS:
    void uploadFile(const QUrl &url, const QString &body = QString());
    void acceptInvitation();
    void forget();
    void sendTypingNotification(bool isTyping);

    /// @param rawText The text as it was typed.
    /// @param cleanedText The text with link to the users.
    void postMessage(const QString &rawText,
                     const QString &cleanedText,
                     Quotient::MessageEventType type = Quotient::MessageEventType::Text,
                     const QString &replyEventId = QString(),
                     const QString &relateToEventId = QString());
    void postHtmlMessage(const QString &text,
                         const QString &html,
                         Quotient::MessageEventType type = Quotient::MessageEventType::Text,
                         const QString &replyEventId = QString(),
                         const QString &relateToEventId = QString());
    void changeAvatar(const QUrl &localFile);
    void toggleReaction(const QString &eventId, const QString &reaction);
    void deleteMessagesByUser(const QString &user, const QString &reason);
};
