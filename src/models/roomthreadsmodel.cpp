// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roomthreadsmodel.h"

#include <Quotient/csapi/threads_list.h>
#include <Quotient/jobs/basejob.h>

#include "eventhandler.h"
#include "threadmodel.h"

RoomThreadsModel::RoomThreadsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void RoomThreadsModel::initializeModel()
{
    if (m_room == nullptr) {
        return;
    }

    auto connection = m_room->connection();
    auto threadsJob = connection->callApi<Quotient::GetThreadRootsJob>(m_room->id());
    connect(threadsJob, &Quotient::BaseJob::success, this, [this, threadsJob]() {
        qDeleteAll(m_threads);
        m_threads.clear();

        beginResetModel();
        m_threads = threadsJob->chunk();

        for (const auto &thread : m_threads) {
            m_threadModels.insert(thread->id(), new ThreadModel(m_room, thread->id(), this));
        }
        endResetModel();
    });
}

NeoChatRoom *RoomThreadsModel::room() const
{
    return m_room;
}

void RoomThreadsModel::setRoom(NeoChatRoom *room)
{
    if (room == m_room) {
        return;
    }
    m_room = room;
    Q_EMIT roomChanged();

    initializeModel();
}

QVariant RoomThreadsModel::data(const QModelIndex &index, int role) const
{
    const auto row = index.row();
    const auto event = m_threads[row].get();

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
        return event->originTimestamp().date() != m_threads[row - 1]->originTimestamp().date();
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

int RoomThreadsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_threads.size();
}

QHash<int, QByteArray> RoomThreadsModel::roleNames() const
{
    return {
        {DelegateTypeRole, "delegateType"},
        {DisplayRole, "displayText"},
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
