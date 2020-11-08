#pragma once

#include <events/encryptionevent.h>
#include <events/redactionevent.h>
#include <events/roomavatarevent.h>
#include <events/roomcreateevent.h>
#include <events/roommemberevent.h>
#include <events/roommessageevent.h>
#include <events/simplestateevents.h>

#include <QObject>
#include <QPointer>
#include <QTimer>

#include "room.h"
#include "neochatuser.h"

using namespace Quotient;

class NeoChatRoom : public Room
{
    Q_OBJECT
    Q_PROPERTY(QVariantList usersTyping READ getUsersTyping NOTIFY typingChanged)
    Q_PROPERTY(QString cachedInput MEMBER m_cachedInput NOTIFY cachedInputChanged)
    Q_PROPERTY(bool hasFileUploading READ hasFileUploading WRITE setHasFileUploading NOTIFY hasFileUploadingChanged)
    Q_PROPERTY(int fileUploadingProgress READ fileUploadingProgress NOTIFY fileUploadingProgressChanged)
    Q_PROPERTY(QString avatarMediaId READ avatarMediaId NOTIFY avatarChanged STORED false)

public:
    explicit NeoChatRoom(Connection *connection, QString roomId, JoinState joinState = {});

    QVariantList getUsersTyping() const;

    QString lastEvent() const;
    bool isEventHighlighted(const Quotient::RoomEvent *e) const;

    QDateTime lastActiveTime() const;

    bool hasFileUploading() const
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

    int fileUploadingProgress() const
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

    Q_INVOKABLE int savedTopVisibleIndex() const;
    Q_INVOKABLE int savedBottomVisibleIndex() const;
    Q_INVOKABLE void saveViewport(int topIndex, int bottomIndex);

    Q_INVOKABLE QVariantList getUsers(const QString &keyword) const;

    Q_INVOKABLE QUrl urlToMxcUrl(QUrl mxcUrl);

    QString avatarMediaId() const;

    QString eventToString(const RoomEvent &evt, Qt::TextFormat format = Qt::PlainText, bool removeReply = true) const;

    Q_INVOKABLE bool containsUser(QString userID) const;

    Q_INVOKABLE bool canSendEvent(const QString &eventType) const;
    Q_INVOKABLE bool canSendState(const QString &eventType) const;

private:
    QString m_cachedInput;
    QSet<const Quotient::RoomEvent *> highlights;

    bool m_hasFileUploading = false;
    int m_fileUploadingProgress = 0;

    void checkForHighlights(const Quotient::TimelineItem &ti);

    void onAddNewTimelineEvents(timeline_iter_t from) override;
    void onAddHistoricalTimelineEvents(rev_iter_t from) override;
    void onRedaction(const RoomEvent &prevEvent, const RoomEvent &after) override;

    static QString markdownToHTML(const QString &plaintext);

private Q_SLOTS:
    void countChanged();

Q_SIGNALS:
    void cachedInputChanged();
    void busyChanged();
    void hasFileUploadingChanged();
    void fileUploadingProgressChanged();
    void backgroundChanged();

public Q_SLOTS:
    void uploadFile(const QUrl &url, const QString &body = "");
    void acceptInvitation();
    void forget();
    void sendTypingNotification(bool isTyping);
    void postArbitaryMessage(const QString &text, Quotient::MessageEventType type = Quotient::MessageEventType::Text, const QString &replyEventId = "");
    void postPlainMessage(const QString &text, Quotient::MessageEventType type = Quotient::MessageEventType::Text, const QString &replyEventId = "");
    void postHtmlMessage(const QString &text, const QString &html, Quotient::MessageEventType type = Quotient::MessageEventType::Text, const QString &replyEventId = "");
    void changeAvatar(QUrl localFile);
    void addLocalAlias(const QString &alias);
    void removeLocalAlias(const QString &alias);
    void toggleReaction(const QString &eventId, const QString &reaction);
};
