// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "searchmodel.h"
#include "events/stickerevent.h"
#include "messageeventmodel.h"
#include "neochatroom.h"
#include "neochatuser.h"
#include <KLocalizedString>
#include <connection.h>

#ifdef QUOTIENT_07
#include <csapi/search.h>
#endif

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
#ifdef QUOTIENT_07
    Q_ASSERT(m_connection);
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
        .orderBy = "recent",
        .eventContext = SearchJob::IncludeEventContext{3, 3, true},
        .includeState = false,
        .groupings = none,

    };

    auto job = m_connection->callApi<SearchJob>(SearchJob::Categories{criteria});
    m_job = job;
    connect(job, &BaseJob::finished, this, [this, job] {
        beginResetModel();
        m_result = job->searchCategories().roomEvents;
        endResetModel();
        setSearching(false);
        m_job = nullptr;
        // TODO error handling
    });
#endif
}

Connection *SearchModel::connection() const
{
    return m_connection;
}

void SearchModel::setConnection(Connection *connection)
{
    m_connection = connection;
    Q_EMIT connectionChanged();
}

QVariant SearchModel::data(const QModelIndex &index, int role) const
{
#ifdef QUOTIENT_07
    auto row = index.row();
    const auto &event = *m_result->results[row].result;
    switch (role) {
    case DisplayRole:
        return m_room->eventToString(*m_result->results[row].result);
    case ShowAuthorRole:
        return true;
    case AuthorRole:
        return m_room->getUser(event.senderId());
    case ShowSectionRole:
        if (row == 0) {
            return true;
        }
        return event.originTimestamp().date() != m_result->results[row - 1].result->originTimestamp().date();
    case SectionRole:
        return renderDate(event.originTimestamp());
    case TimeRole:
        return event.originTimestamp();
    case ShowReactionsRole:
        return false;
    case ShowReadMarkersRole:
        return false;
    case ReplyAuthorRole:
        if (const auto &replyPtr = m_room->getReplyForEvent(event)) {
            return m_room->getUser(static_cast<NeoChatUser *>(m_room->user(replyPtr->senderId())));
        } else {
            return m_room->getUser(nullptr);
        }
    case ReplyRole:
        if (role == ReplyRole) {
            auto replyPtr = m_room->getReplyForEvent(event);
            if (!replyPtr) {
                return {};
            }

            MessageEventModel::DelegateType type;
            if (auto e = eventCast<const RoomMessageEvent>(replyPtr)) {
                switch (e->msgtype()) {
                case MessageEventType::Emote:
                    type = MessageEventModel::DelegateType::Emote;
                    break;
                case MessageEventType::Notice:
                    type = MessageEventModel::DelegateType::Notice;
                    break;
                case MessageEventType::Image:
                    type = MessageEventModel::DelegateType::Image;
                    break;
                case MessageEventType::Audio:
                    type = MessageEventModel::DelegateType::Audio;
                    break;
                case MessageEventType::Video:
                    type = MessageEventModel::DelegateType::Video;
                    break;
                default:
                    if (e->hasFileContent()) {
                        type = MessageEventModel::DelegateType::File;
                        break;
                    }
                    type = MessageEventModel::DelegateType::Message;
                }

            } else if (is<const StickerEvent>(*replyPtr)) {
                type = MessageEventModel::DelegateType::Sticker;
            } else {
                type = MessageEventModel::DelegateType::Other;
            }

            return QVariantMap{
                {"display", m_room->eventToString(*replyPtr, Qt::RichText)},
                {"type", type},
            };
        }
    case IsPendingRole:
        return false;
    case ShowLinkPreviewRole:
        return false;
    case IsReplyRole:
        return !event.contentJson()["m.relates_to"].toObject()["m.in_reply_to"].toObject()["event_id"].toString().isEmpty();
    case HighlightRole:
        return !m_room->isDirectChat() && m_room->isEventHighlighted(&event);
    case EventIdRole:
        return event.id();
    case ReplyIdRole:
        return event.contentJson()["m.relates_to"].toObject()["m.in_reply_to"].toObject()["event_id"].toString();
    }
    return MessageEventModel::DelegateType::Message;
#endif
    return {};
}

int SearchModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
#ifdef QUOTIENT_07
    if (m_result.has_value()) {
        return m_result->results.size();
    }
#endif
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
        {ShowAuthorRole, "showAuthor"},
        {EventIdRole, "eventId"},
        {ExcessReadMarkersRole, "excessReadMarkers"},
        {HighlightRole, "isHighlighted"},
        {ReadMarkersString, "readMarkersString"},
        {PlainTextRole, "plainText"},
        {VerifiedRole, "verified"},
        {ReplyAuthorRole, "replyAuthor"},
        {ProgressInfoRole, "progressInfo"},
        {IsReplyRole, "isReply"},
        {ShowReactionsRole, "showReactions"},
        {ReplyRole, "reply"},
        {ReactionRole, "reaction"},
        {ReplyMediaInfoRole, "replyMediaInfo"},
        {ReadMarkersRole, "readMarkers"},
        {IsPendingRole, "isPending"},
        {ShowReadMarkersRole, "showReadMarkers"},
        {ReplyIdRole, "replyId"},
        {MimeTypeRole, "mimeType"},
        {ShowLinkPreviewRole, "showLinkPreview"},
        {LinkPreviewRole, "linkPreview"},
        {SourceRole, "source"},
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

#ifdef QUOTIENT_07
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
        Q_EMIT dataChanged(index(row, 0), index(row, 0), {ReplyRole, ReplyMediaInfoRole, ReplyAuthorRole});
    });
#endif
}

// TODO deduplicate with messageeventmodel
QString renderDate(const QDateTime &timestamp)
{
    auto date = timestamp.toLocalTime().date();
    if (date == QDate::currentDate()) {
        return i18n("Today");
    }
    if (date == QDate::currentDate().addDays(-1)) {
        return i18n("Yesterday");
    }
    if (date == QDate::currentDate().addDays(-2)) {
        return i18n("The day before yesterday");
    }
    if (date > QDate::currentDate().addDays(-7)) {
        return date.toString("dddd");
    }

    return QLocale::system().toString(date, QLocale::ShortFormat);
}

bool SearchModel::searching() const
{
    return m_searching;
}

void SearchModel::setSearching(bool searching)
{
    m_searching = searching;
    Q_EMIT searchingChanged();
}
