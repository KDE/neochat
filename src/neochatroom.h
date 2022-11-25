// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <room.h>

#include <QCache>
#include <QObject>
#include <QTextCursor>

#include <qcoro/task.h>

class PollHandler;
class NeoChatUser;

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
    Q_PROPERTY(bool isInvite READ isInvite NOTIFY isInviteChanged)
    Q_PROPERTY(QString joinRule READ joinRule WRITE setJoinRule NOTIFY joinRuleChanged)
    Q_PROPERTY(QString historyVisibility READ historyVisibility WRITE setHistoryVisibility NOTIFY historyVisibilityChanged)
    Q_PROPERTY(QString htmlSafeDisplayName READ htmlSafeDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(PushNotificationState::State pushNotificationState MEMBER m_currentPushNotificationState WRITE setPushNotificationState NOTIFY
                   pushNotificationStateChanged)

    // Due to problems with QTextDocument, unlike the other properties here, chatBoxText is *not* used to store the text when switching rooms
    Q_PROPERTY(QString chatBoxText READ chatBoxText WRITE setChatBoxText NOTIFY chatBoxTextChanged)
    Q_PROPERTY(QString chatBoxReplyId READ chatBoxReplyId WRITE setChatBoxReplyId NOTIFY chatBoxReplyIdChanged)
    Q_PROPERTY(QString chatBoxEditId READ chatBoxEditId WRITE setChatBoxEditId NOTIFY chatBoxEditIdChanged)
    Q_PROPERTY(NeoChatUser *chatBoxReplyUser READ chatBoxReplyUser NOTIFY chatBoxReplyIdChanged)
    Q_PROPERTY(QString chatBoxReplyMessage READ chatBoxReplyMessage NOTIFY chatBoxReplyIdChanged)
    Q_PROPERTY(NeoChatUser *chatBoxEditUser READ chatBoxEditUser NOTIFY chatBoxEditIdChanged)
    Q_PROPERTY(QString chatBoxEditMessage READ chatBoxEditMessage NOTIFY chatBoxEditIdChanged)
    Q_PROPERTY(QString chatBoxAttachmentPath READ chatBoxAttachmentPath WRITE setChatBoxAttachmentPath NOTIFY chatBoxAttachmentPathChanged)
    Q_PROPERTY(bool canEncryptRoom READ canEncryptRoom NOTIFY canEncryptRoomChanged)

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
    [[nodiscard]] const Quotient::RoomEvent *lastEvent(bool ignoreStateEvent = false) const;

    /// Convenient way to get the last event but in a string format.
    ///
    /// \see lastEvent
    /// \see lastEventIsSpoiler
    [[nodiscard]] QString lastEventToString() const;

    /// Convenient way to check if the last event looks like it has spoilers.
    ///
    /// \see lastEvent
    /// \see lastEventToString
    [[nodiscard]] bool lastEventIsSpoiler() const;

    /// Convenient way to get the QDateTime of the last event.
    ///
    /// \see lastEvent
    [[nodiscard]] QDateTime lastActiveTime();

    /// Get subtitle text for room
    ///
    /// Fetches last event and removes markdown formatting
    /// \see lastEventToString
    [[nodiscard]] QString subtitleText();

    [[nodiscard]] bool isSpace();

    bool isEventHighlighted(const Quotient::RoomEvent *e) const;

    [[nodiscard]] QString joinRule() const;
    void setJoinRule(const QString &joinRule);

    [[nodiscard]] QString historyVisibility() const;
    void setHistoryVisibility(const QString &historyVisibilityRule);

    [[nodiscard]] bool hasFileUploading() const
    {
        return m_hasFileUploading;
    }
    void setHasFileUploading(bool value)
    {
        if (value == m_hasFileUploading) {
            return;
        }
        m_hasFileUploading = value;
        Q_EMIT hasFileUploadingChanged();
    }

    [[nodiscard]] int fileUploadingProgress() const
    {
        return m_fileUploadingProgress;
    }
    void setFileUploadingProgress(int value)
    {
        if (m_fileUploadingProgress == value) {
            return;
        }
        m_fileUploadingProgress = value;
        Q_EMIT fileUploadingProgressChanged();
    }

    [[nodiscard]] bool readMarkerLoaded() const;

    Q_INVOKABLE [[nodiscard]] int savedTopVisibleIndex() const;
    Q_INVOKABLE [[nodiscard]] int savedBottomVisibleIndex() const;
    Q_INVOKABLE void saveViewport(int topIndex, int bottomIndex);

    Q_INVOKABLE [[nodiscard]] QVariantList getUsers(const QString &keyword, int limit = -1) const;
    Q_INVOKABLE [[nodiscard]] QVariantMap getUser(const QString &userID) const;

    Q_INVOKABLE QUrl urlToMxcUrl(const QUrl &mxcUrl);

    [[nodiscard]] QString avatarMediaId() const;

    [[nodiscard]] QString eventToString(const Quotient::RoomEvent &evt, Qt::TextFormat format = Qt::PlainText, bool removeReply = true) const;

    Q_INVOKABLE [[nodiscard]] bool containsUser(const QString &userID) const;
    Q_INVOKABLE [[nodiscard]] bool isUserBanned(const QString &user) const;

    Q_INVOKABLE [[nodiscard]] bool canSendEvent(const QString &eventType) const;
    Q_INVOKABLE [[nodiscard]] bool canSendState(const QString &eventType) const;

    bool isInvite() const;

    Q_INVOKABLE QString htmlSafeName() const;
    Q_INVOKABLE QString htmlSafeDisplayName() const;
    Q_INVOKABLE void clearInvitationNotification();
    Q_INVOKABLE void reportEvent(const QString &eventId, const QString &reason);

    Q_INVOKABLE void setPushNotificationState(PushNotificationState::State state);

    Q_INVOKABLE void download(const QString &eventId, const QUrl &localFilename = {});

    QString chatBoxText() const;
    void setChatBoxText(const QString &text);

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

    QString savedText() const;
    void setSavedText(const QString &savedText);

    bool canEncryptRoom() const;

    Q_INVOKABLE bool downloadTempFile(const QString &eventId);

#ifdef QUOTIENT_07
    Q_INVOKABLE PollHandler *poll(const QString &eventId);
#endif

#ifndef QUOTIENT_07
    Q_INVOKABLE QString htmlSafeMemberName(const QString &userId) const
    {
        return safeMemberName(userId).toHtmlEscaped();
    }
#endif

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
    QString m_chatBoxReplyId;
    QString m_chatBoxEditId;
    QString m_chatBoxAttachmentPath;
    QVector<Mention> m_mentions;
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
    void chatBoxReplyIdChanged();
    void chatBoxEditIdChanged();
    void chatBoxAttachmentPathChanged();
    void canEncryptRoomChanged();
    void joinRuleChanged();
    void historyVisibilityChanged();

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
    void addLocalAlias(const QString &alias);
    void removeLocalAlias(const QString &alias);
    void toggleReaction(const QString &eventId, const QString &reaction);
    void deleteMessagesByUser(const QString &user, const QString &reason);
};
