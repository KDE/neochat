// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "timelinemessagemodel.h"
#include "events/pollevent.h"
#include "messagemodel_logging.h"

#include <Quotient/events/reactionevent.h>

using namespace Quotient;

TimelineMessageModel::TimelineMessageModel(QObject *parent)
    : MessageModel(parent)
{
    connect(this, &TimelineMessageModel::roomChanged, this, &TimelineMessageModel::connectNewRoom);
}

void TimelineMessageModel::connectNewRoom()
{
    if (m_room) {
        m_lastReadEventIndex = QPersistentModelIndex(QModelIndex());
        m_room->setDisplayed();

        for (auto event = m_room->messageEvents().begin(); event != m_room->messageEvents().end(); ++event) {
            Q_EMIT newEventAdded(event->get());
        }

        if (m_room->timelineSize() < 10 && !m_room->allHistoryLoaded()) {
            m_room->getPreviousContent(50);
        }

        connect(m_room, &Room::aboutToAddNewMessages, this, [this](RoomEventsRange events) {
            if (!m_resetting) {
                m_initialized = true;
                beginInsertRows({}, timelineServerIndex(), timelineServerIndex() + int(events.size()) - 1);
            }
        });
        connect(m_room, &Room::aboutToAddHistoricalMessages, this, [this](RoomEventsRange events) {
            if (!m_resetting) {
                m_initialized = true;
                beginInsertRows({}, rowCount(), rowCount() + int(events.size()) - 1);
            }
        });
        connect(m_room, &Room::addedMessages, this, [this](int lowest, int biggest) {
            if (m_initialized) {
                for (int i = lowest; i <= biggest; ++i) {
                    const auto event = m_room->findInTimeline(i)->event();
                    Q_EMIT newEventAdded(event);
                }

                endInsertRows();
            }
            if (!m_lastReadEventIndex.isValid()) {
                // no read marker, so see if we need to create one.
                moveReadMarker(m_room->lastFullyReadEventId());
            }
            if (biggest < m_room->maxTimelineIndex()) {
                auto rowBelowInserted = m_room->maxTimelineIndex() - biggest + timelineServerIndex() - 1;
                refreshEventRoles(rowBelowInserted, {ContentModelRole});
            }
            for (auto i = m_room->maxTimelineIndex() - biggest; i <= m_room->maxTimelineIndex() - lowest; ++i) {
                refreshLastUserEvents(i);
            }
        });
        connect(m_room, &Room::pendingEventAdded, this, [this](const Quotient::RoomEvent *event) {
            Q_EMIT newEventAdded(event);
            // Don't scroll the timeline view for reactions.
            if (!event->is<ReactionEvent>()) {
                Q_EMIT newLocalUserEventAdded();
            }
            if (!m_resetting) {
                beginInsertRows({}, 0, 0);
                endInsertRows();
            }
        });
        connect(m_room, &Room::pendingEventAboutToMerge, this, [this](RoomEvent *, int i) {
            Q_EMIT dataChanged(index(i, 0), index(i, 0), {IsPendingRole});
            if (i == 0) {
                return; // No need to move anything, just refresh
            }

            m_movingEvent = true;
            // Reverse i because row 0 is bottommost in the model
            const auto row = timelineServerIndex() - i - 1;
            beginMoveRows({}, row, row, {}, timelineServerIndex());
        });
        connect(m_room, &Room::pendingEventMerged, this, [this] {
            if (m_movingEvent) {
                endMoveRows();
                m_movingEvent = false;
            }
            fullEventRefresh(timelineServerIndex());
            refreshLastUserEvents(0);
            if (timelineServerIndex() > 0) { // Refresh below, see #312
                refreshEventRoles(timelineServerIndex() - 1, {ContentModelRole});
            }
        });
        connect(m_room, &Room::pendingEventChanged, this, &TimelineMessageModel::fullEventRefresh);
        connect(m_room, &Room::pendingEventAboutToDiscard, this, [this](int i) {
            beginRemoveRows({}, i, i);
        });
        connect(m_room, &Room::pendingEventDiscarded, this, &TimelineMessageModel::endRemoveRows);
        connect(m_room, &Room::fullyReadMarkerMoved, this, [this](const QString &fromEventId, const QString &toEventId) {
            Q_UNUSED(fromEventId);
            moveReadMarker(toEventId);
        });
        connect(m_room, &Room::replacedEvent, this, [this](const RoomEvent *newEvent) {
            Q_EMIT newEventAdded(newEvent);
        });
        connect(m_room, &Room::updatedEvent, this, [this](const QString &eventId) {
            if (eventId.isEmpty()) { // How did we get here?
                return;
            }
            const auto eventIt = m_room->findInTimeline(eventId);
            if (eventIt != m_room->historyEdge()) {
                Q_EMIT newEventAdded(eventIt->event());
            }
            refreshEventRoles(eventId, {Qt::DisplayRole});
        });
        connect(m_room, &Room::changed, this, [this](Room::Changes changes) {
            if (changes.testFlag(Quotient::Room::Change::Other)) {
                // this is slow
                for (auto it = m_room->messageEvents().rbegin(); it != m_room->messageEvents().rend(); ++it) {
                    Q_EMIT newEventAdded(it->event());
                }
            }
        });
        connect(m_room->connection(), &Connection::ignoredUsersListChanged, this, [this] {
            beginResetModel();
            endResetModel();
        });

        qCDebug(Message) << "Connected to room" << m_room->id() << "as" << m_room->localMember().id();
    }

    // After reset put a read marker in if required.
    // This is needed when changing back to a room that has already loaded messages.
    if (m_room) {
        moveReadMarker(m_room->lastFullyReadEventId());
    }
}

int TimelineMessageModel::timelineServerIndex() const
{
    return m_room ? int(m_room->pendingEvents().size()) : 0;
}

std::optional<std::reference_wrapper<const RoomEvent>> TimelineMessageModel::getEventForIndex(QModelIndex index) const
{
    const auto row = index.row();
    bool isPending = row < timelineServerIndex();
    const auto timelineIt = m_room->messageEvents().crbegin()
        + std::max(0, row - timelineServerIndex() - (m_lastReadEventIndex.isValid() && m_lastReadEventIndex.row() < row ? 1 : 0));
    const auto pendingIt = m_room->pendingEvents().crbegin() + std::min(row, timelineServerIndex());
    return isPending ? **pendingIt : **timelineIt;
}

int TimelineMessageModel::rowCount(const QModelIndex &parent) const
{
    if (!m_room || parent.isValid()) {
        return 0;
    }

    return int(m_room->pendingEvents().size()) + m_room->timelineSize() + (m_lastReadEventIndex.isValid() ? 1 : 0);
}

bool TimelineMessageModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_room && !m_room->eventsHistoryJob() && !m_room->allHistoryLoaded();
}

void TimelineMessageModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (m_room) {
        m_room->getPreviousContent(20);
    }
}

#include "moc_timelinemessagemodel.cpp"
