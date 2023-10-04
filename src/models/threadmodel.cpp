// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "threadmodel.h"

#include <Quotient/csapi/relations.h>
#include <Quotient/jobs/basejob.h>

#include "eventhandler.h"
#include "neochatroom.h"

ThreadModel::ThreadModel(const NeoChatRoom *room, const QString &threadRootId, QObject *parent)
    : QAbstractListModel(parent)
    , m_room(room)
    , m_threadRootId(threadRootId)
{
    intializeModel();
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

    auto connection = m_room->connection();
    auto threadEventsJob = connection->callApi<Quotient::GetRelatingEventsWithRelTypeJob>(m_room->id(), m_threadRootId, QLatin1String("m.thread"));
    connect(threadEventsJob, &Quotient::BaseJob::success, this, [this, threadEventsJob]() {
        beginResetModel();
        m_events = threadEventsJob->chunk();
        endResetModel();
        m_loading = false;
    });
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
    case ShowReactionsRole:
        return false;
    case ShowReadMarkersRole:
        return false;
    case IsReplyRole:
        return eventHandler.hasReply();
    case ReplyIdRole:
        return eventHandler.hasReply();
    case ReplyAuthorRole:
        return eventHandler.getReplyAuthor();
    case ReplyDelegateTypeRole:
        return eventHandler.getReplyDelegateType();
    case ReplyDisplayRole:
        return eventHandler.getReplyRichBody();
    case ReplyMediaInfoRole:
        return eventHandler.getReplyMediaInfo();
    case IsPendingRole:
        return false;
    case ShowLinkPreviewRole:
        return false;
    case HighlightRole:
        return eventHandler.isHighlighted();
    case EventIdRole:
        return eventHandler.getId();
    }
    return DelegateType::Message;
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
        {ReactionRole, "reaction"},
        {ReadMarkersRole, "readMarkers"},
        {IsPendingRole, "isPending"},
        {ShowReadMarkersRole, "showReadMarkers"},
        {MimeTypeRole, "mimeType"},
        {ShowLinkPreviewRole, "showLinkPreview"},
        {LinkPreviewRole, "linkPreview"},
    };
}
#include "moc_threadmodel.cpp"
