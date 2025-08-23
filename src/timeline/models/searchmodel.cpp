// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "searchmodel.h"

using namespace Quotient;

SearchModel::SearchModel(QObject *parent)
    : MessageModel(parent)
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
    if (!m_allRooms) {
        filter.rooms = QStringList{m_room->id()};
    }
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
        clearEventObjects();

        beginResetModel();
        m_result = job->searchCategories().roomEvents;

        if (m_result.has_value()) {
            for (const auto &result : m_result.value().results) {
                Q_EMIT newEventAdded(result.result.get());
            }
        }

        endResetModel();
        setSearching(false);
        m_job = nullptr;
        // TODO error handling
    });
}

std::optional<std::reference_wrapper<const RoomEvent>> SearchModel::getEventForIndex(QModelIndex index) const
{
    if (!m_result.has_value()) {
        return std::nullopt;
    }

    return *m_result.value().results.at(index.row()).result.get();
}

int SearchModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_result.has_value()) {
        return m_result->results.size();
    }
    return 0;
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

bool SearchModel::allRooms() const
{
    return m_allRooms;
}

void SearchModel::setAllRooms(bool allRooms)
{
    if (m_allRooms == allRooms) {
        return;
    }
    m_allRooms = allRooms;
    Q_EMIT allRoomsChanged();
}

#include "moc_searchmodel.cpp"
