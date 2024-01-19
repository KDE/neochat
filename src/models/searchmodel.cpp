// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "searchmodel.h"

#include "eventhandler.h"
#include "messageeventmodel.h"
#include "neochatroom.h"

#include <QGuiApplication>

#include <Quotient/connection.h>
#include <Quotient/events/stickerevent.h>

#include <KLocalizedString>

using namespace Quotient;

// TODO search only in the current room

SearchModel::SearchModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QString SearchModel::searchText() const
{
    return m_searchText;
}

void SearchModel::setSearchText(const QString &searchText)
{
    m_searchText = searchText;
    Q_EMIT searchTextChanged();
}

void SearchModel::search()
{
    Q_ASSERT(m_room);
    setSearching(true);
    if (m_job) {
        m_job->abandon();
        m_job = nullptr;
    }

    RoomEventFilter filter;
    filter.unreadThreadNotifications = none;
    filter.lazyLoadMembers = true;
    filter.includeRedundantMembers = false;
    filter.notRooms = QStringList();
    filter.rooms = QStringList{m_room->id()};
    filter.containsUrl = false;

    SearchJob::RoomEventsCriteria criteria{
        .searchTerm = m_searchText,
        .keys = {},
        .filter = filter,
        .orderBy = "recent"_ls,
        .eventContext = SearchJob::IncludeEventContext{3, 3, true},
        .includeState = false,
        .groupings = none,

    };

    auto job = m_room->connection()->callApi<SearchJob>(SearchJob::Categories{criteria});
    m_job = job;
    connect(job, &BaseJob::finished, this, [this, job] {
        beginResetModel();
        m_result = job->searchCategories().roomEvents;
        endResetModel();
        setSearching(false);
        m_job = nullptr;
        // TODO error handling
    });
}

QVariant SearchModel::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    const auto &event = *m_result->results[row].result;

    EventHandler eventHandler;
    eventHandler.setRoom(m_room);
    eventHandler.setEvent(&event);

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
        return event.originTimestamp().date() != m_result->results[row - 1].result->originTimestamp().date();
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
    case IsThreadedRole:
        return eventHandler.isThreaded();
    case ThreadRootRole:
        return eventHandler.threadRoot();
    }
    return DelegateType::Message;
}

int SearchModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_result.has_value()) {
        return m_result->results.size();
    }
    return 0;
}

QHash<int, QByteArray> SearchModel::roleNames() const
{
    return {
        {DelegateTypeRole, "delegateType"},
        {DisplayRole, "display"},
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
        {IsThreadedRole, "isThreaded"},
        {ThreadRootRole, "threadRoot"},
    };
}

NeoChatRoom *SearchModel::room() const
{
    return m_room;
}

void SearchModel::setRoom(NeoChatRoom *room)
{
    if (m_room) {
        disconnect(m_room, nullptr, this, nullptr);
    }
    m_room = room;
    Q_EMIT roomChanged();

    connect(m_room, &NeoChatRoom::replyLoaded, this, [this](const auto &eventId, const auto &replyId) {
        Q_UNUSED(replyId);
        const auto &results = m_result->results;
        auto it = std::find_if(results.begin(), results.end(), [eventId](const auto &event) {
            return event.result->id() == eventId;
        });
        if (it == results.end()) {
            return;
        }
        auto row = it - results.begin();
        Q_EMIT dataChanged(index(row, 0), index(row, 0), {ReplyDelegateTypeRole, ReplyDisplayRole, ReplyMediaInfoRole, ReplyAuthorRole});
    });
}

bool SearchModel::searching() const
{
    return m_searching;
}

bool SearchModel::event(QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange) {
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {AuthorRole, ReadMarkersRole});
    }
    return QObject::event(event);
}

void SearchModel::setSearching(bool searching)
{
    m_searching = searching;
    Q_EMIT searchingChanged();
}

#include "moc_searchmodel.cpp"
