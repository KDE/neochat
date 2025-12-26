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
    if (!m_room) {
        qWarning() << "SearchModel: No room";
        return;
    }

    if (m_job) {
        m_job->abandon();
        m_job = nullptr;
    }

    // early-return case: the user sets the text to nothing, and we simply clear the results
    if (m_searchText.isEmpty()) {
        clearEventObjects();

        beginResetModel();
        m_nextBatch.clear();
        m_results.clear();
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

    auto job = m_room->connection()->callApi<SearchJob>(SearchJob::Categories{criteria}, m_nextBatch);
    m_job = job;
    connect(job, &BaseJob::finished, this, [this, job] {
        clearEventObjects();

        auto results = job->searchCategories().roomEvents;
        if (results.has_value()) {
            beginInsertRows({}, rowCount({}), rowCount({}) + int(results->results.size()) - 1);
            for (const auto &result : results.value().results) {
                Q_EMIT newEventAdded(result.result.get());
            }
            std::move(results->results.begin(), results->results.end(), std::back_inserter(m_results));
            endInsertRows();

            m_nextBatch = results->nextBatch;
        } else {
            m_nextBatch.clear();
        }

        setSearching(false);
        m_job = nullptr;
        // TODO error handling
    });
}

void SearchModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    search();
}

bool SearchModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return !m_nextBatch.isEmpty() && !searching();
}

std::optional<std::reference_wrapper<const RoomEvent>> SearchModel::getEventForIndex(QModelIndex index) const
{
    if (m_results.empty()) {
        return std::nullopt;
    }

    return *m_results.at(index.row()).result.get();
}

int SearchModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_results.size();
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
