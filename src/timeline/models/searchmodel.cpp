// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "searchmodel.h"

using namespace Quotient;

// TODO search only in the current room

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

    if (m_job) {
        m_job->abandon();
        m_job = nullptr;
    }

    // early-return case: the user sets the text to nothing, and we simply clear the results
    if (m_searchText.isEmpty()) {
        clearEventObjects();

        beginResetModel();
        m_result = std::nullopt;
        endResetModel();
        return;
    }

    setSearching(true);

    RoomEventFilter filter;
    filter.unreadThreadNotifications = std::nullopt;
    filter.lazyLoadMembers = true;
    filter.includeRedundantMembers = false;
    filter.notRooms = QStringList();
    filter.rooms = QStringList{m_room->id()};
    filter.containsUrl = false;
    if (!m_senderId.isEmpty()) {
        filter.senders = {m_senderId};
    }

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

QString SearchModel::senderId() const
{
    return m_senderId;
}

void SearchModel::setSenderId(const QString &sender)
{
    m_senderId = sender;
    Q_EMIT senderIdChanged();
}

#include "moc_searchmodel.cpp"
