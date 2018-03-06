#include "matriqueroom.h"

#include "user.h"
#include "events/roommessageevent.h"

using namespace QMatrixClient;

MatriqueRoom::MatriqueRoom(Connection* connection, QString roomId,
                               JoinState joinState)
    : Room(connection, roomId, joinState)
{
    connect(this, &MatriqueRoom::notificationCountChanged, this, &MatriqueRoom::countChanged);
    connect(this, &MatriqueRoom::highlightCountChanged, this, &MatriqueRoom::countChanged);
}

const QString& MatriqueRoom::cachedInput() const
{
    return m_cachedInput;
}

void MatriqueRoom::setCachedInput(const QString& input)
{
    m_cachedInput = input;
}

bool MatriqueRoom::isEventHighlighted(RoomEvent* e) const
{
    return highlights.contains(e);
}

int MatriqueRoom::savedTopVisibleIndex() const
{
    return firstDisplayedMarker() == timelineEdge() ? 0 :
                firstDisplayedMarker() - messageEvents().rbegin();
}

int MatriqueRoom::savedBottomVisibleIndex() const
{
    return lastDisplayedMarker() == timelineEdge() ? 0 :
                lastDisplayedMarker() - messageEvents().rbegin();
}

void MatriqueRoom::saveViewport(int topIndex, int bottomIndex)
{
    if (bottomIndex == savedBottomVisibleIndex() &&
            (bottomIndex == 0 || topIndex == savedTopVisibleIndex()))
        return;
    if (bottomIndex == 0)
    {
        qDebug() << "Saving viewport as the latest available";
        setFirstDisplayedEventId({}); setLastDisplayedEventId({});
        return;
    }
    qDebug() << "Saving viewport:" << topIndex << "thru" << bottomIndex;
    setFirstDisplayedEvent(maxTimelineIndex() - topIndex);
    setLastDisplayedEvent(maxTimelineIndex() - bottomIndex);
}

void MatriqueRoom::countChanged()
{
    if(displayed() && !hasUnreadMessages())
    {
        resetNotificationCount();
        resetHighlightCount();
    }
}

void MatriqueRoom::onAddNewTimelineEvents(timeline_iter_t from)
{
    std::for_each(from, messageEvents().cend(),
                  [this] (const TimelineItem& ti) { checkForHighlights(ti); });
}

void MatriqueRoom::onAddHistoricalTimelineEvents(rev_iter_t from)
{
    std::for_each(from, messageEvents().crend(),
                  [this] (const TimelineItem& ti) { checkForHighlights(ti); });
}

void MatriqueRoom::checkForHighlights(const QMatrixClient::TimelineItem& ti)
{
    auto localUserId = localUser()->id();
    if (ti->senderId() == localUserId)
        return;
    if (ti->type() == EventType::RoomMessage)
    {
        auto* rme = static_cast<const RoomMessageEvent*>(ti.event());
        if (rme->plainBody().contains(localUserId) ||
                rme->plainBody().contains(roomMembername(localUserId)))
            highlights.insert(ti.event());
    }
}
