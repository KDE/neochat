// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "searchmodel.h"

#include "enums/delegatetype.h"
#include "eventhandler.h"
#include "models/messagecontentmodel.h"
#include "neochatroom.h"

#include <QGuiApplication>

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
    filter.unreadThreadNotifications = std::nullopt;
    filter.lazyLoadMembers = true;
    filter.includeRedundantMembers = false;
    filter.notRooms = QStringList();
    filter.rooms = QStringList{m_room->id()};
    filter.containsUrl = false;

    SearchJob::RoomEventsCriteria criteria{
        .searchTerm = m_searchText,
        .keys = {},
        .filter = filter,
        .orderBy = "recent"_L1,
        .eventContext = SearchJob::IncludeEventContext{3, 3, true},
        .includeState = false,
        .groupings = std::nullopt,

    };

    auto job = m_room->connection()->callApi<SearchJob>(SearchJob::Categories{criteria});
    m_job = job;
    connect(job, &BaseJob::finished, this, [this, job] {
        beginResetModel();
        m_memberObjects.clear();
        m_result = job->searchCategories().roomEvents;

        if (m_result.has_value()) {
            for (const auto &result : m_result.value().results) {
                if (!m_memberObjects.contains(result.result->senderId())) {
                    m_memberObjects[result.result->senderId()] = std::unique_ptr<NeochatRoomMember>(new NeochatRoomMember(m_room, result.result->senderId()));
                }
            }
        }

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

    switch (role) {
    case AuthorRole:
        return QVariant::fromValue<NeochatRoomMember *>(m_memberObjects.at(event.senderId()).get());
    case ShowSectionRole:
        if (row == 0) {
            return true;
        }
        return event.originTimestamp().date() != m_result->results[row - 1].result->originTimestamp().date();
    case SectionRole:
        return EventHandler::timeString(&event, true);
    case ShowReactionsRole:
        return false;
    case ShowReadMarkersRole:
        return false;
    case IsPendingRole:
        return false;
    case HighlightRole:
        return EventHandler::isHighlighted(m_room, &event);
    case EventIdRole:
        return event.displayId();
    case IsThreadedRole:
        if (auto roomMessageEvent = eventCast<const RoomMessageEvent>(&event)) {
            return roomMessageEvent->isThreaded();
        }
        return {};
    case ThreadRootRole:
        if (auto roomMessageEvent = eventCast<const RoomMessageEvent>(&event); roomMessageEvent->isThreaded()) {
            return roomMessageEvent->threadRootEventId();
        }
        return {};
    case ContentModelRole: {
        if (!event.isStateEvent()) {
            return QVariant::fromValue<MessageContentModel *>(new MessageContentModel(m_room, event.id()));
        }
        if (event.isStateEvent()) {
            if (event.matrixType() == u"org.matrix.msc3672.beacon_info"_s) {
                return QVariant::fromValue<MessageContentModel *>(new MessageContentModel(m_room, event.id()));
            }
        }
        return {};
    }
    case IsEditableRole: {
        return false;
    }
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
        {AuthorRole, "author"},
        {ShowSectionRole, "showSection"},
        {SectionRole, "section"},
        {EventIdRole, "eventId"},
        {ExcessReadMarkersRole, "excessReadMarkers"},
        {HighlightRole, "isHighlighted"},
        {ReadMarkersString, "readMarkersString"},
        {VerifiedRole, "verified"},
        {ShowReactionsRole, "showReactions"},
        {ReactionRole, "reaction"},
        {ReadMarkersRole, "readMarkers"},
        {IsPendingRole, "isPending"},
        {ShowReadMarkersRole, "showReadMarkers"},
        {IsThreadedRole, "isThreaded"},
        {ThreadRootRole, "threadRoot"},
        {ContentModelRole, "contentModel"},
        {IsEditableRole, "isEditable"},
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
