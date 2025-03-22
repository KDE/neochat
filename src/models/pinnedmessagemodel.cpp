// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "pinnedmessagemodel.h"

#include "enums/delegatetype.h"
#include "eventhandler.h"
#include "models/messagecontentmodel.h"
#include "neochatroom.h"

#include <QGuiApplication>

#include <KLocalizedString>

using namespace Quotient;

PinnedMessageModel::PinnedMessageModel(QObject *parent)
    : MessageModel(parent)
{
    connect(this, &MessageModel::roomChanged, this, &PinnedMessageModel::fill);
}

bool PinnedMessageModel::loading() const
{
    return m_loading;
}

int PinnedMessageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_pinnedEvents.size();
}

std::optional<std::reference_wrapper<const Quotient::RoomEvent>> PinnedMessageModel::getEventForIndex(const QModelIndex index) const
{
    if (static_cast<size_t>(index.row()) >= m_pinnedEvents.size() || index.row() < 0) {
        return std::nullopt;
    }
    return std::reference_wrapper{*m_pinnedEvents[index.row()].get()};
}

void PinnedMessageModel::setLoading(bool loading)
{
    m_loading = loading;
    Q_EMIT loadingChanged();
}

void PinnedMessageModel::fill()
{
    if (!m_room) {
        return;
    }

    const auto events = m_room->pinnedEventIds();

    for (const auto &event : std::as_const(events)) {
        auto job = m_room->connection()->callApi<GetOneRoomEventJob>(m_room->id(), event);
        connect(job, &BaseJob::success, this, [this, job] {
            beginInsertRows({}, m_pinnedEvents.size(), m_pinnedEvents.size());
            m_pinnedEvents.push_back(std::move(fromJson<event_ptr_tt<RoomEvent>>(job->jsonData())));
            Q_EMIT newEventAdded(m_pinnedEvents.back().get());
            endInsertRows();
        });
    }
}

#include "moc_pinnedmessagemodel.cpp"
