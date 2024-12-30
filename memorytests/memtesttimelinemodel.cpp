// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "memtesttimelinemodel.h"

#include <Quotient/events/eventcontent.h>
#include <Quotient/events/roommessageevent.h>

using namespace Quotient;

MemTestTimelineModel::MemTestTimelineModel(QObject *parent)
    : MessageModel(parent)
{
    beginResetModel();
    m_connection = Connection::makeMockConnection(u"@bob:example.org"_s);
    m_room = new MemTestRoom(m_connection, u"#memtestroom:example.org"_s, u"memtest-sync.json"_s);

    for (const auto &eventIt : m_room->messageEvents()) {
        Q_EMIT newEventAdded(eventIt.event());
    }

    endResetModel();
}

std::optional<std::reference_wrapper<const RoomEvent>> MemTestTimelineModel::getEventForIndex(QModelIndex index) const
{
    return *m_room->messageEvents().at(index.row()).event();
}

int MemTestTimelineModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_room->messageEvents().size();
}

#include "moc_memtesttimelinemodel.cpp"
