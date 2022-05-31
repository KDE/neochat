// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "searchmodel.h"
#include "messageeventmodel.h"
#include "neochatroom.h"
#include "neochatuser.h"
#include <KLocalizedString>
#include <connection.h>

#include <csapi/search.h>

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
    Q_ASSERT(m_connection);
    setSearching(true);
    if (m_job) {
        m_job->abandon();
        m_job = nullptr;
    }

    SearchJob::RoomEventsCriteria criteria{
        .searchTerm = m_searchText,
        .keys = {},
        .filter =
            RoomEventFilter{
                .unreadThreadNotifications = none,
                .lazyLoadMembers = true,
                .includeRedundantMembers = false,
                .notRooms = {},
                .rooms = {m_room->id()},
                .containsUrl = false,
            },
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
    auto row = index.row();
    const auto &event = *m_result->results[row].result;
    switch (role) {
    case DisplayRole:
        return m_room->eventToString(*m_result->results[row].result);
    case ShowAuthorRole:
        return true;
    case AuthorRole:
        return QVariantMap{
            {"isLocalUser", event.senderId() == m_room->localUser()->id()},
            {"id", event.senderId()},
            {"avatarMediaId", m_connection->user(event.senderId())->avatarMediaId(m_room)},
            {"avatarUrl", m_connection->user(event.senderId())->avatarUrl(m_room)},
            {"displayName", m_connection->user(event.senderId())->displayname(m_room)},
            {"display", m_connection->user(event.senderId())->name()},
            {"color", dynamic_cast<NeoChatUser *>(m_connection->user(event.senderId()))->color()},
            {"object", QVariant::fromValue(m_connection->user(event.senderId()))},
        };
    case ShowSectionRole:
        if (row == 0) {
            return true;
        }
        return event.originTimestamp().date() != m_result->results[row - 1].result->originTimestamp().date();
    case SectionRole:
        return renderDate(event.originTimestamp());
    case TimeRole:
        return event.originTimestamp();
    }
    return MessageEventModel::DelegateType::Message;
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
        {EventTypeRole, "eventType"},
        {DisplayRole, "display"},
        {AuthorRole, "author"},
        {ShowSectionRole, "showSection"},
        {SectionRole, "section"},
        {TimeRole, "time"},
        {ShowAuthorRole, "showAuthor"},
    };
}

NeoChatRoom *SearchModel::room() const
{
    return m_room;
}

void SearchModel::setRoom(NeoChatRoom *room)
{
    m_room = room;
    Q_EMIT roomChanged();
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
