// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "joinrulesevent.h"
#include <events/encryptionevent.h>
#include <events/redactionevent.h>
#include <events/roomavatarevent.h>
#include <events/roomcreateevent.h>
#include <events/roommemberevent.h>
#include <events/roommessageevent.h>
#include <events/simplestateevents.h>
#include <room.h>

#include <QObject>
#include <QPointer>
#include <QTimer>

#include <qcoro/task.h>

#include "neochatuser.h"

using namespace Quotient;

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

class NeoChatRoom : public Room
{
    Q_OBJECT
    Q_PROPERTY(QVariantList usersTyping READ getUsersTyping NOTIFY typingChanged)
    Q_PROPERTY(QString cachedInput MEMBER m_cachedInput NOTIFY cachedInputChanged)
    Q_PROPERTY(bool hasFileUploading READ hasFileUploading WRITE setHasFileUploading NOTIFY hasFileUploadingChanged)
    Q_PROPERTY(int fileUploadingProgress READ fileUploadingProgress NOTIFY fileUploadingProgressChanged)
    Q_PROPERTY(QString avatarMediaId READ avatarMediaId NOTIFY avatarChanged STORED false)
    Q_PROPERTY(bool readMarkerLoaded READ readMarkerLoaded NOTIFY readMarkerLoadedChanged)
    Q_PROPERTY(QDateTime lastActiveTime READ lastActiveTime NOTIFY lastActiveTimeChanged)
    Q_PROPERTY(bool isInvite READ isInvite NOTIFY isInviteChanged)
    Q_PROPERTY(QString joinRule READ joinRule CONSTANT)
    Q_PROPERTY(QString htmlSafeDisplayName READ htmlSafeDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(PushNotificationState::State pushNotificationState MEMBER m_currentPushNotificationState WRITE setPushNotificationState NOTIFY
                   pushNotificationStateChanged)

public:
    explicit NeoChatRoom(Connection *connection, QString roomId, JoinState joinState = {});

    [[nodiscard]] QVariantList getUsersTyping() const;

    /// Get the interesting last event.
    ///
    /// This function respect the showLeaveJoinEvent setting and discard
    /// other not interesting events. This function can return an empty pointer
    /// when the room is empty of RoomMessageEvent.
    [[nodiscard]] const RoomMessageEvent *lastEvent(bool ignoreStateEvent = false) const;

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

    [[nodiscard]] QString eventToString(const RoomEvent &evt, Qt::TextFormat format = Qt::PlainText, bool removeReply = true) const;

    Q_INVOKABLE [[nodiscard]] bool containsUser(const QString &userID) const;
    Q_INVOKABLE [[nodiscard]] bool isUserBanned(const QString &user) const;

    Q_INVOKABLE [[nodiscard]] bool canSendEvent(const QString &eventType) const;
    Q_INVOKABLE [[nodiscard]] bool canSendState(const QString &eventType) const;

    bool isInvite() const;

    Q_INVOKABLE QString htmlSafeName() const;
    Q_INVOKABLE QString htmlSafeDisplayName() const;
    Q_INVOKABLE void clearInvitationNotification();

    Q_INVOKABLE void setPushNotificationState(PushNotificationState::State state);

#ifndef QUOTIENT_07
    Q_INVOKABLE QString htmlSafeMemberName(const QString &userId) const
    {
        return safeMemberName(userId).toHtmlEscaped();
    }
#endif

private:
    QString m_cachedInput;
    QSet<const Quotient::RoomEvent *> highlights;

    bool m_hasFileUploading = false;
    int m_fileUploadingProgress = 0;

    PushNotificationState::State m_currentPushNotificationState = PushNotificationState::State::Unknown;
    bool m_pushNotificationStateUpdating = false;

    void checkForHighlights(const Quotient::TimelineItem &ti);

    void onAddNewTimelineEvents(timeline_iter_t from) override;
    void onAddHistoricalTimelineEvents(rev_iter_t from) override;
    void onRedaction(const RoomEvent &prevEvent, const RoomEvent &after) override;

    static QString markdownToHTML(const QString &markdown);
    QCoro::Task<void> doDeleteMessagesByUser(const QString &user);
    QCoro::Task<void> doUploadFile(QUrl url, QString body = QString());

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

public Q_SLOTS:
    void uploadFile(const QUrl &url, const QString &body = QString());
    void acceptInvitation();
    void forget();
    void sendTypingNotification(bool isTyping);
    QString preprocessText(const QString &text);

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
    void deleteMessagesByUser(const QString &user);
};
