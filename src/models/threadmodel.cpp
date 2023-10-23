// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "threadmodel.h"

#include <Quotient/csapi/relations.h>
#include <Quotient/events/stickerevent.h>
#include <Quotient/jobs/basejob.h>

#include "eventhandler.h"
#include "neochatroom.h"

ThreadModel::ThreadModel(const NeoChatRoom *room, const QString &threadRootId, QObject *parent)
    : QAbstractListModel(parent)
    , m_room(room)
    , m_threadRootId(threadRootId)
{
    connect(this, &ThreadModel::rowsInserted, this, [this]() {
        if (m_loading) {
            m_loading = false;
            Q_EMIT loadingChanged();
        }
    });
    intializeModel();
}

QString ThreadModel::threadRootId() const
{
    return m_threadRootId;
}

bool ThreadModel::loading() const
{
    return m_loading;
}

void ThreadModel::intializeModel()
{
    if (m_threadRootId.isEmpty() || m_room == nullptr) {
        return;
    }
    m_events.clear();
    *m_nextBatch = QString();
    m_loading = true;
}

QVariant ThreadModel::data(const QModelIndex &index, int role) const
{
    const auto row = index.row();
    const auto event = m_events[row].get();

    EventHandler eventHandler;
    eventHandler.setRoom(m_room);
    eventHandler.setEvent(event);

    switch (role) {
    case DisplayRole:
        return eventHandler.getRichBody();
    case DelegateTypeRole:
        return eventHandler.getDelegateType();
    case ShowAuthorRole:
        return true;
    case AuthorRole:
        return eventHandler.getAuthor();
    case ShowSectionRole:
        if (row == 0) {
            return true;
        }
        return event->originTimestamp().date() != m_events[row - 1]->originTimestamp().date();
    case SectionRole:
        return eventHandler.getTimeString(true);
    case TimeRole:
        return eventHandler.getTime();
    case TimeStringRole:
        return eventHandler.getTimeString(false);
    case ProgressInfoRole:
        if (auto e = eventCast<const Quotient::RoomMessageEvent>(event)) {
            if (e->hasFileContent()) {
                return QVariant::fromValue(m_room->fileTransferInfo(e->id()));
            }
        }
        if (auto e = eventCast<const Quotient::StickerEvent>(event)) {
            return QVariant::fromValue(m_room->fileTransferInfo(e->id()));
        }
        break;
    case ShowReactionsRole:
        return false;
    case ShowReadMarkersRole:
        return false;
    case IsReplyRole:
        return eventHandler.hasReply(false);
    case ReplyIdRole:
        return eventHandler.getReplyId();
    case ReplyAuthorRole:
        return eventHandler.getReplyAuthor();
    case ReplyDelegateTypeRole:
        return eventHandler.getReplyDelegateType();
    case ReplyDisplayRole:
        return eventHandler.getReplyRichBody();
    case ReplyMediaInfoRole:
        return eventHandler.getReplyMediaInfo();
    case IsThreadedRole:
        return eventHandler.isThreaded();
    case ThreadRootRole:
        return eventHandler.threadRoot();
    case IsPendingRole:
        return false;
    case ShowLinkPreviewRole:
        return false;
    case HighlightRole:
        return eventHandler.isHighlighted();
    case EventIdRole:
        return eventHandler.getId();
    }
    return {};
}

int ThreadModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_events.size();
}

QHash<int, QByteArray> ThreadModel::roleNames() const
{
    return {
        {DisplayRole, "display"},
        {DelegateTypeRole, "delegateType"},
        {AuthorRole, "author"},
        {ShowSectionRole, "showSection"},
        {SectionRole, "section"},
        {TimeRole, "time"},
        {TimeStringRole, "timeString"},
        {ShowAuthorRole, "showAuthor"},
        {EventIdRole, "eventId"},
        {ExcessReadMarkersRole, "excessReadMarkers"},
        {HighlightRole, "isHighlighted"},
        {ReadMarkersString, "readMarkersString"},
        {PlainTextRole, "plainText"},
        {VerifiedRole, "verified"},
        {ProgressInfoRole, "progressInfo"},
        {ShowReactionsRole, "showReactions"},
        {IsReplyRole, "isReply"},
        {ReplyAuthorRole, "replyAuthor"},
        {ReplyIdRole, "replyId"},
        {ReplyDelegateTypeRole, "replyDelegateType"},
        {ReplyDisplayRole, "replyDisplay"},
        {ReplyMediaInfoRole, "replyMediaInfo"},
        {IsThreadedRole, "isThreaded"},
        {ThreadRootRole, "threadRoot"},
        {ReactionRole, "reaction"},
        {ReadMarkersRole, "readMarkers"},
        {IsPendingRole, "isPending"},
        {ShowReadMarkersRole, "showReadMarkers"},
        {MimeTypeRole, "mimeType"},
        {ShowLinkPreviewRole, "showLinkPreview"},
        {LinkPreviewRole, "linkPreview"},
    };
}

bool ThreadModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_nextBatch.has_value();
}

void ThreadModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (!m_currentJob) {
        auto connection = m_room->connection();
        auto threadEventsJob =
            connection->callApi<Quotient::GetRelatingEventsWithRelTypeJob>(m_room->id(), m_threadRootId, QLatin1String("m.thread"), *m_nextBatch);
        connect(threadEventsJob, &Quotient::BaseJob::success, this, [this, threadEventsJob]() {
            auto newEvents = threadEventsJob->chunk();
            beginInsertRows(QModelIndex(), rowCount(), rowCount() + newEvents.size() - 1);
            m_events.insert(m_events.end(), std::make_move_iterator(newEvents.begin()), std::make_move_iterator(newEvents.end()));
            endInsertRows();
            const auto newNextBatch = threadEventsJob->nextBatch();
            if (!newNextBatch.isEmpty() && *m_nextBatch != newNextBatch) {
                *m_nextBatch = newNextBatch;
            } else {
                m_nextBatch.reset();
            }

            m_currentJob.clear();
        });
    }
}
#include "moc_threadmodel.cpp"
