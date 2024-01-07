// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "messageeventmodel.h"
#include "linkpreviewer.h"
#include "messageeventmodel_logging.h"

#include "neochatconfig.h"

#include <Quotient/connection.h>
#include <Quotient/csapi/rooms.h>
#include <Quotient/events/redactionevent.h>
#include <Quotient/events/stickerevent.h>
#include <Quotient/user.h>

#include <QDebug>
#include <QGuiApplication>
#include <QTimeZone>

#include <KLocalizedString>

#include "enums/delegatetype.h"
#include "eventhandler.h"
#include "events/pollevent.h"
#include "models/reactionmodel.h"
#include "texthandler.h"

using namespace Quotient;

QHash<int, QByteArray> MessageEventModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[DelegateTypeRole] = "delegateType";
    roles[PlainText] = "plainText";
    roles[EventIdRole] = "eventId";
    roles[TimeRole] = "time";
    roles[TimeStringRole] = "timeString";
    roles[SectionRole] = "section";
    roles[AuthorRole] = "author";
    roles[HighlightRole] = "isHighlighted";
    roles[SpecialMarksRole] = "marks";
    roles[ProgressInfoRole] = "progressInfo";
    roles[ShowLinkPreviewRole] = "showLinkPreview";
    roles[LinkPreviewRole] = "linkPreview";
    roles[MediaInfoRole] = "mediaInfo";
    roles[IsReplyRole] = "isReply";
    roles[ReplyAuthor] = "replyAuthor";
    roles[ReplyIdRole] = "replyId";
    roles[ReplyDelegateTypeRole] = "replyDelegateType";
    roles[ReplyDisplayRole] = "replyDisplay";
    roles[ReplyMediaInfoRole] = "replyMediaInfo";
    roles[IsThreadedRole] = "isThreaded";
    roles[ThreadRootRole] = "threadRoot";
    roles[ShowAuthorRole] = "showAuthor";
    roles[ShowSectionRole] = "showSection";
    roles[ReadMarkersRole] = "readMarkers";
    roles[ExcessReadMarkersRole] = "excessReadMarkers";
    roles[ReadMarkersStringRole] = "readMarkersString";
    roles[ShowReadMarkersRole] = "showReadMarkers";
    roles[ReactionRole] = "reaction";
    roles[ShowReactionsRole] = "showReactions";
    roles[VerifiedRole] = "verified";
    roles[AuthorDisplayNameRole] = "authorDisplayName";
    roles[IsRedactedRole] = "isRedacted";
    roles[GenericDisplayRole] = "genericDisplay";
    roles[IsPendingRole] = "isPending";
    roles[LatitudeRole] = "latitude";
    roles[LongitudeRole] = "longitude";
    roles[AssetRole] = "asset";
    roles[PollHandlerRole] = "pollHandler";
    return roles;
}

MessageEventModel::MessageEventModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &MessageEventModel::modelAboutToBeReset, this, [this]() {
        resetting = true;
    });
    connect(this, &MessageEventModel::modelReset, this, [this]() {
        resetting = false;
    });
}

NeoChatRoom *MessageEventModel::room() const
{
    return m_currentRoom;
}

void MessageEventModel::setRoom(NeoChatRoom *room)
{
    if (room == m_currentRoom) {
        return;
    }

    beginResetModel();
    if (m_currentRoom) {
        m_currentRoom->disconnect(this);
        m_linkPreviewers.clear();
        m_reactionModels.clear();
    }

    m_currentRoom = room;
    Q_EMIT roomChanged();
    if (room) {
        m_lastReadEventIndex = QPersistentModelIndex(QModelIndex());
        room->setDisplayed();

        for (auto event = m_currentRoom->messageEvents().begin(); event != m_currentRoom->messageEvents().end(); ++event) {
            if (const auto &roomMessageEvent = &*event->viewAs<RoomMessageEvent>()) {
                createEventObjects(roomMessageEvent);
            }
            if (event->event()->is<PollStartEvent>()) {
                m_currentRoom->createPollHandler(eventCast<const PollStartEvent>(event->event()));
            }
        }

        if (m_currentRoom->timelineSize() < 10 && !room->allHistoryLoaded()) {
            room->getPreviousContent(50);
        }
        lastReadEventId = room->lastFullyReadEventId();
        connect(m_currentRoom, &NeoChatRoom::replyLoaded, this, [this](const auto &eventId, const auto &replyId) {
            Q_UNUSED(replyId);
            auto row = eventIdToRow(eventId);
            if (row == -1) {
                return;
            }
            Q_EMIT dataChanged(index(row, 0), index(row, 0), {ReplyDelegateTypeRole, ReplyDisplayRole, ReplyMediaInfoRole, ReplyAuthor});
        });

        connect(m_currentRoom, &Room::aboutToAddNewMessages, this, [this](RoomEventsRange events) {
            for (auto &&event : events) {
                const RoomMessageEvent *message = dynamic_cast<RoomMessageEvent *>(event.get());

                if (message != nullptr) {
                    createEventObjects(message);
                    if (NeoChatConfig::self()->showFancyEffects()) {
                        QString planBody = message->plainBody();
                        // snowflake
                        const QString snowlakeEmoji = QString::fromUtf8("\xE2\x9D\x84");
                        if (planBody.contains(snowlakeEmoji)) {
                            Q_EMIT fancyEffectsReasonFound(QStringLiteral("snowflake"));
                        }
                        // fireworks
                        const QString fireworksEmoji = QString::fromUtf8("\xF0\x9F\x8E\x86");
                        if (planBody.contains(fireworksEmoji)) {
                            Q_EMIT fancyEffectsReasonFound(QStringLiteral("fireworks"));
                        }
                        // sparkler
                        const QString sparklerEmoji = QString::fromUtf8("\xF0\x9F\x8E\x87");
                        if (planBody.contains(sparklerEmoji)) {
                            Q_EMIT fancyEffectsReasonFound(QStringLiteral("fireworks"));
                        }
                        // party pooper
                        const QString partyEmoji = QString::fromUtf8("\xF0\x9F\x8E\x89");
                        if (planBody.contains(partyEmoji)) {
                            Q_EMIT fancyEffectsReasonFound(QStringLiteral("confetti"));
                        }
                        // confetti ball
                        const QString confettiEmoji = QString::fromUtf8("\xF0\x9F\x8E\x8A");
                        if (planBody.contains(confettiEmoji)) {
                            Q_EMIT fancyEffectsReasonFound(QStringLiteral("confetti"));
                        }
                    }
                }
                if (event->is<PollStartEvent>()) {
                    m_currentRoom->createPollHandler(eventCast<const PollStartEvent>(event.get()));
                }
            }
            m_initialized = true;
            beginInsertRows({}, timelineBaseIndex(), timelineBaseIndex() + int(events.size()) - 1);
        });
        connect(m_currentRoom, &Room::aboutToAddHistoricalMessages, this, [this](RoomEventsRange events) {
            for (auto &event : events) {
                if (const auto &roomMessageEvent = dynamic_cast<RoomMessageEvent *>(event.get())) {
                    createEventObjects(roomMessageEvent);
                }
                if (event->is<PollStartEvent>()) {
                    m_currentRoom->createPollHandler(eventCast<const PollStartEvent>(event.get()));
                }
            }
            if (rowCount() > 0) {
                rowBelowInserted = rowCount() - 1; // See #312
            }
            m_initialized = true;
            beginInsertRows({}, rowCount(), rowCount() + int(events.size()) - 1);
        });
        connect(m_currentRoom, &Room::addedMessages, this, [this](int lowest, int biggest) {
            if (m_initialized) {
                endInsertRows();
            }
            if (!m_lastReadEventIndex.isValid()) {
                // no read marker, so see if we need to create one.
                moveReadMarker(m_currentRoom->lastFullyReadEventId());
            }
            if (biggest < m_currentRoom->maxTimelineIndex()) {
                auto rowBelowInserted = m_currentRoom->maxTimelineIndex() - biggest + timelineBaseIndex() - 1;
                refreshEventRoles(rowBelowInserted, {ShowAuthorRole});
            }
            for (auto i = m_currentRoom->maxTimelineIndex() - biggest; i <= m_currentRoom->maxTimelineIndex() - lowest; ++i) {
                refreshLastUserEvents(i);
            }
        });
        connect(m_currentRoom, &Room::pendingEventAboutToAdd, this, [this] {
            m_initialized = true;
            beginInsertRows({}, 0, 0);
        });
        connect(m_currentRoom, &Room::pendingEventAdded, this, &MessageEventModel::endInsertRows);
        connect(m_currentRoom, &Room::pendingEventAboutToMerge, this, [this](RoomEvent *, int i) {
            Q_EMIT dataChanged(index(i, 0), index(i, 0), {IsPendingRole});
            if (i == 0) {
                return; // No need to move anything, just refresh
            }

            movingEvent = true;
            // Reverse i because row 0 is bottommost in the model
            const auto row = timelineBaseIndex() - i - 1;
            beginMoveRows({}, row, row, {}, timelineBaseIndex());
        });
        connect(m_currentRoom, &Room::pendingEventMerged, this, [this] {
            if (movingEvent) {
                endMoveRows();
                movingEvent = false;
            }
            refreshRow(timelineBaseIndex()); // Refresh the looks
            refreshLastUserEvents(0);
            if (timelineBaseIndex() > 0) { // Refresh below, see #312
                refreshEventRoles(timelineBaseIndex() - 1, {ShowAuthorRole});
            }
        });
        connect(m_currentRoom, &Room::pendingEventChanged, this, &MessageEventModel::refreshRow);
        connect(m_currentRoom, &Room::pendingEventAboutToDiscard, this, [this](int i) {
            beginRemoveRows({}, i, i);
        });
        connect(m_currentRoom, &Room::pendingEventDiscarded, this, &MessageEventModel::endRemoveRows);
        connect(m_currentRoom, &Room::fullyReadMarkerMoved, this, [this](const QString &fromEventId, const QString &toEventId) {
            Q_UNUSED(fromEventId);
            moveReadMarker(toEventId);
        });
        connect(m_currentRoom, &Room::replacedEvent, this, [this](const RoomEvent *newEvent) {
            refreshLastUserEvents(refreshEvent(newEvent->id()) - timelineBaseIndex());
            const RoomMessageEvent *message = eventCast<const RoomMessageEvent>(newEvent);
            if (message != nullptr) {
                createEventObjects(message);
            }
        });
        connect(m_currentRoom, &Room::updatedEvent, this, [this](const QString &eventId) {
            if (eventId.isEmpty()) { // How did we get here?
                return;
            }
            const auto eventIt = m_currentRoom->findInTimeline(eventId);
            if (eventIt != m_currentRoom->historyEdge()) {
                if (const auto &event = dynamic_cast<const RoomMessageEvent *>(&**eventIt)) {
                    createEventObjects(event);
                }
                if (eventIt->event()->is<PollStartEvent>()) {
                    m_currentRoom->createPollHandler(eventCast<const PollStartEvent>(eventIt->event()));
                }
            }
            refreshEventRoles(eventId, {Qt::DisplayRole});
        });
        connect(m_currentRoom, &Room::changed, this, [this]() {
            for (auto it = m_currentRoom->messageEvents().rbegin(); it != m_currentRoom->messageEvents().rend(); ++it) {
                auto event = it->event();
                refreshEventRoles(event->id(), {ReadMarkersRole, ReadMarkersStringRole, ExcessReadMarkersRole});
            }
        });
        connect(m_currentRoom, &Room::newFileTransfer, this, &MessageEventModel::refreshEvent);
        connect(m_currentRoom, &Room::fileTransferProgress, this, &MessageEventModel::refreshEvent);
        connect(m_currentRoom, &Room::fileTransferCompleted, this, &MessageEventModel::refreshEvent);
        connect(m_currentRoom, &Room::fileTransferFailed, this, &MessageEventModel::refreshEvent);
        connect(m_currentRoom->connection(), &Connection::ignoredUsersListChanged, this, [this] {
            beginResetModel();
            endResetModel();
        });
        qCDebug(MessageEvent) << "Connected to room" << room->id() << "as" << room->localUser()->id();
    } else {
        lastReadEventId.clear();
    }
    endResetModel();

    // After reset put a read marker in if required.
    // This is needed when changing back to a room that has already loaded messages.
    if (room) {
        moveReadMarker(m_currentRoom->lastFullyReadEventId());
    }
}

int MessageEventModel::refreshEvent(const QString &eventId)
{
    return refreshEventRoles(eventId);
}

void MessageEventModel::refreshRow(int row)
{
    refreshEventRoles(row);
}

int MessageEventModel::timelineBaseIndex() const
{
    return m_currentRoom ? int(m_currentRoom->pendingEvents().size()) : 0;
}

void MessageEventModel::refreshEventRoles(int row, const QList<int> &roles)
{
    const auto idx = index(row);
    Q_EMIT dataChanged(idx, idx, roles);
}

void MessageEventModel::moveReadMarker(const QString &toEventId)
{
    const auto timelineIt = m_currentRoom->findInTimeline(toEventId);
    if (timelineIt == m_currentRoom->historyEdge()) {
        return;
    }
    int newRow = int(timelineIt - m_currentRoom->messageEvents().rbegin()) + timelineBaseIndex();

    if (!m_lastReadEventIndex.isValid()) {
        // Not valid index means we don't display any marker yet, in this case
        // we create the new index and insert the row in case the read marker
        // need to be displayed.
        if (newRow > timelineBaseIndex()) {
            // The user didn't read all the messages yet.
            m_initialized = true;
            beginInsertRows({}, newRow, newRow);
            m_lastReadEventIndex = QPersistentModelIndex(index(newRow, 0));
            endInsertRows();
            return;
        }
        // The user read all the messages and we didn't display any read marker yet
        // => do nothing
        return;
    }
    if (newRow <= timelineBaseIndex()) {
        // The user read all the messages => remove read marker
        beginRemoveRows({}, m_lastReadEventIndex.row(), m_lastReadEventIndex.row());
        m_lastReadEventIndex = QModelIndex();
        endRemoveRows();
        return;
    }

    // The user didn't read all the messages yet but moved the reader marker.
    beginMoveRows({}, m_lastReadEventIndex.row(), m_lastReadEventIndex.row(), {}, newRow);
    m_lastReadEventIndex = QPersistentModelIndex(index(newRow, 0));
    endMoveRows();
}

int MessageEventModel::refreshEventRoles(const QString &id, const QList<int> &roles)
{
    // On 64-bit platforms, difference_type for std containers is long long
    // but Qt uses int throughout its interfaces; hence casting to int below.
    int row = -1;
    // First try pendingEvents because it is almost always very short.
    const auto pendingIt = m_currentRoom->findPendingEvent(id);
    if (pendingIt != m_currentRoom->pendingEvents().end()) {
        row = int(pendingIt - m_currentRoom->pendingEvents().begin());
    } else {
        const auto timelineIt = m_currentRoom->findInTimeline(id);
        if (timelineIt == m_currentRoom->historyEdge()) {
            return -1;
        }
        row = int(timelineIt - m_currentRoom->messageEvents().rbegin()) + timelineBaseIndex();
        if (data(index(row, 0), DelegateTypeRole).toInt() == DelegateType::ReadMarker || data(index(row, 0), DelegateTypeRole).toInt() == DelegateType::Other) {
            row++;
        }
    }
    refreshEventRoles(row, roles);
    return row;
}

inline bool hasValidTimestamp(const Quotient::TimelineItem &ti)
{
    return ti->originTimestamp().isValid();
}

QDateTime MessageEventModel::makeMessageTimestamp(const Quotient::Room::rev_iter_t &baseIt) const
{
    const auto &timeline = m_currentRoom->messageEvents();
    auto ts = baseIt->event()->originTimestamp();
    if (ts.isValid()) {
        return ts;
    }

    // The event is most likely redacted or just invalid.
    // Look for the nearest date around and slap zero time to it.
    using Quotient::TimelineItem;
    auto rit = std::find_if(baseIt, timeline.rend(), hasValidTimestamp);
    if (rit != timeline.rend()) {
        return {rit->event()->originTimestamp().date(), {0, 0}, Qt::LocalTime};
    };
    auto it = std::find_if(baseIt.base(), timeline.end(), hasValidTimestamp);
    if (it != timeline.end()) {
        return {it->event()->originTimestamp().date(), {0, 0}, Qt::LocalTime};
    };

    // What kind of room is that?..
    qCCritical(MessageEvent) << "No valid timestamps in the room timeline!";
    return {};
}

void MessageEventModel::refreshLastUserEvents(int baseTimelineRow)
{
    if (!m_currentRoom || m_currentRoom->timelineSize() <= baseTimelineRow) {
        return;
    }

    const auto &timelineBottom = m_currentRoom->messageEvents().rbegin();
    const auto &lastSender = (*(timelineBottom + baseTimelineRow))->senderId();
    const auto limit = timelineBottom + std::min(baseTimelineRow + 10, m_currentRoom->timelineSize());
    for (auto it = timelineBottom + std::max(baseTimelineRow - 10, 0); it != limit; ++it) {
        if ((*it)->senderId() == lastSender) {
            auto idx = index(it - timelineBottom);
            Q_EMIT dataChanged(idx, idx);
        }
    }
}

int MessageEventModel::rowCount(const QModelIndex &parent) const
{
    if (!m_currentRoom || parent.isValid()) {
        return 0;
    }

    return int(m_currentRoom->pendingEvents().size()) + m_currentRoom->timelineSize() + (m_lastReadEventIndex.isValid() ? 1 : 0);
}

bool MessageEventModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_currentRoom && !m_currentRoom->eventsHistoryJob() && !m_currentRoom->allHistoryLoaded();
}

void MessageEventModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (m_currentRoom) {
        m_currentRoom->getPreviousContent(20);
    }
}

static LinkPreviewer *emptyLinkPreview = new LinkPreviewer;

QVariant MessageEventModel::data(const QModelIndex &idx, int role) const
{
    if (!checkIndex(idx, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return {};
    }
    const auto row = idx.row();

    if (!m_currentRoom || row < 0 || row >= rowCount()) {
        return {};
    };

    bool isPending = row < timelineBaseIndex();

    if (m_lastReadEventIndex.row() == row) {
        switch (role) {
        case DelegateTypeRole:
            return DelegateType::ReadMarker;
        case TimeRole: {
            const QDateTime eventDate = data(index(m_lastReadEventIndex.row() + 1, 0), TimeRole).toDateTime().toLocalTime();
            const KFormat format;
            return format.formatRelativeDateTime(eventDate, QLocale::ShortFormat);
        }
        case SpecialMarksRole:
            // Check if all the earlier events in the timeline are hidden. If so hide this.
            for (auto r = row - 1; r >= 0; --r) {
                const auto specialMark = index(r).data(SpecialMarksRole);
                if (!(specialMark == EventStatus::Hidden || specialMark == EventStatus::Replaced)) {
                    return EventStatus::Normal;
                }
            }
            return EventStatus::Hidden;
        }
        return {};
    }

    const auto timelineIt = m_currentRoom->messageEvents().crbegin()
        + std::max(0, row - timelineBaseIndex() - (m_lastReadEventIndex.isValid() && m_lastReadEventIndex.row() < row ? 1 : 0));
    const auto pendingIt = m_currentRoom->pendingEvents().crbegin() + std::min(row, timelineBaseIndex());
    const auto &evt = isPending ? **pendingIt : **timelineIt;

    EventHandler eventHandler;
    eventHandler.setRoom(m_currentRoom);
    eventHandler.setEvent(&evt);

    if (role == Qt::DisplayRole) {
        if (evt.isRedacted()) {
            auto reason = evt.redactedBecause()->reason();
            return (reason.isEmpty()) ? i18n("<i>[This message was deleted]</i>")
                                      : i18n("<i>[This message was deleted: %1]</i>", evt.redactedBecause()->reason());
        }
        return eventHandler.getRichBody();
    }

    if (role == GenericDisplayRole) {
        return eventHandler.getGenericBody();
    }

    if (role == PlainText) {
        return eventHandler.getPlainBody();
    }

    if (role == DelegateTypeRole) {
        return eventHandler.getDelegateType();
    }

    if (role == AuthorRole) {
        return eventHandler.getAuthor(isPending);
    }

    if (role == HighlightRole) {
        return eventHandler.isHighlighted();
    }

    if (role == SpecialMarksRole) {
        if (isPending) {
            // A pending event with an m.new_content key will be merged into the
            // original event so don't show.
            if (evt.contentJson().contains("m.new_content"_ls)) {
                return EventStatus::Hidden;
            }
            return pendingIt->deliveryStatus();
        }

        if (eventHandler.isHidden()) {
            return EventStatus::Hidden;
        }

        return EventStatus::Normal;
    }

    if (role == EventIdRole) {
        return eventHandler.getId();
    }

    if (role == ProgressInfoRole) {
        if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
            if (e->hasFileContent()) {
                return QVariant::fromValue(m_currentRoom->fileTransferInfo(e->id()));
            }
        }
        if (auto e = eventCast<const StickerEvent>(&evt)) {
            return QVariant::fromValue(m_currentRoom->fileTransferInfo(e->id()));
        }
    }

    if (role == TimeRole) {
        auto lastUpdated = isPending ? pendingIt->lastUpdated() : QDateTime();
        return eventHandler.getTime(isPending, lastUpdated);
    }

    if (role == TimeStringRole) {
        auto lastUpdated = isPending ? pendingIt->lastUpdated() : QDateTime();
        return eventHandler.getTimeString(false, QLocale::ShortFormat, isPending, lastUpdated);
    }

    if (role == SectionRole) {
        auto lastUpdated = isPending ? pendingIt->lastUpdated() : QDateTime();
        return eventHandler.getTimeString(true, QLocale::ShortFormat, isPending, lastUpdated);
    }

    if (role == ShowLinkPreviewRole) {
        return m_linkPreviewers.contains(evt.id());
    }

    if (role == LinkPreviewRole) {
        if (m_linkPreviewers.contains(evt.id())) {
            return QVariant::fromValue<LinkPreviewer *>(m_linkPreviewers[evt.id()].data());
        } else {
            return QVariant::fromValue<LinkPreviewer *>(emptyLinkPreview);
        }
    }

    if (role == MediaInfoRole) {
        return eventHandler.getMediaInfo();
    }

    if (role == IsReplyRole) {
        return eventHandler.hasReply();
    }

    if (role == ReplyIdRole) {
        return eventHandler.getReplyId();
    }

    if (role == ReplyDelegateTypeRole) {
        return eventHandler.getReplyDelegateType();
    }

    if (role == ReplyAuthor) {
        return eventHandler.getReplyAuthor();
    }

    if (role == ReplyDisplayRole) {
        return eventHandler.getReplyRichBody();
    }

    if (role == ReplyMediaInfoRole) {
        return eventHandler.getReplyMediaInfo();
    }

    if (role == IsThreadedRole) {
        return eventHandler.isThreaded();
    }

    if (role == ThreadRootRole) {
        return eventHandler.threadRoot();
    }

    if (role == ShowAuthorRole) {
        for (auto r = row + 1; r < rowCount(); ++r) {
            auto i = index(r);
            // Note !itemData(i).empty() is a check for instances where rows have been removed, e.g. when the read marker is moved.
            // While the row is removed the subsequent row indexes are not changed so we need to skip over the removed index.
            // See - https://doc.qt.io/qt-5/qabstractitemmodel.html#beginRemoveRows
            if (data(i, SpecialMarksRole) != EventStatus::Hidden && !itemData(i).empty()) {
                return data(i, AuthorRole) != data(idx, AuthorRole) || data(i, DelegateTypeRole) == DelegateType::State
                    || data(i, TimeRole).toDateTime().msecsTo(data(idx, TimeRole).toDateTime()) > 600000
                    || data(i, TimeRole).toDateTime().toLocalTime().date().day() != data(idx, TimeRole).toDateTime().toLocalTime().date().day()
                    // FIXME: This should not be necessary; the proper fix is to calculate this role in MessageFilterModel with the knowledge about the filtered
                    // events.
                    || data(i, IsRedactedRole).toBool();
            }
        }

        return true;
    }

    if (role == ShowSectionRole) {
        for (auto r = row + 1; r < rowCount(); ++r) {
            auto i = index(r);
            // Note !itemData(i).empty() is a check for instances where rows have been removed, e.g. when the read marker is moved.
            // While the row is removed the subsequent row indexes are not changed so we need to skip over the removed index.
            // See - https://doc.qt.io/qt-5/qabstractitemmodel.html#beginRemoveRows
            if (data(i, SpecialMarksRole) != EventStatus::Hidden && !itemData(i).empty()) {
                const auto day = data(idx, TimeRole).toDateTime().toLocalTime().date().dayOfYear();
                const auto previousEventDay = data(i, TimeRole).toDateTime().toLocalTime().date().dayOfYear();
                return day != previousEventDay;
            }
        }

        return false;
    }

    if (role == LatitudeRole) {
        return eventHandler.getLatitude();
    }

    if (role == LongitudeRole) {
        return eventHandler.getLongitude();
    }

    if (role == AssetRole) {
        return eventHandler.getLocationAssetType();
    }

    if (role == ReadMarkersRole) {
        return eventHandler.getReadMarkers();
    }

    if (role == ExcessReadMarkersRole) {
        return eventHandler.getNumberExcessReadMarkers();
    }

    if (role == ReadMarkersStringRole) {
        return eventHandler.getReadMarkersString();
    }

    if (role == ShowReadMarkersRole) {
        return eventHandler.hasReadMarkers();
    }

    if (role == ReactionRole) {
        if (m_reactionModels.contains(evt.id())) {
            return QVariant::fromValue<ReactionModel *>(m_reactionModels[evt.id()].data());
        } else {
            return QVariantList();
        }
    }

    if (role == ShowReactionsRole) {
        return m_reactionModels.contains(evt.id());
    }

    if (role == VerifiedRole) {
        if (evt.originalEvent()) {
            auto encrypted = dynamic_cast<const EncryptedEvent *>(evt.originalEvent());
            Q_ASSERT(encrypted);
            return m_currentRoom->connection()->isVerifiedSession(encrypted->sessionId().toLatin1());
        }
        return false;
    }

    if (role == AuthorDisplayNameRole) {
        return eventHandler.getAuthorDisplayName(isPending);
    }

    if (role == IsRedactedRole) {
        return evt.isRedacted();
    }

    if (role == IsPendingRole) {
        return row < static_cast<int>(m_currentRoom->pendingEvents().size());
    }

    if (role == PollHandlerRole) {
        return QVariant::fromValue<PollHandler *>(m_currentRoom->poll(evt.id()));
    }

    return {};
}

int MessageEventModel::eventIdToRow(const QString &eventID) const
{
    const auto it = m_currentRoom->findInTimeline(eventID);
    if (it == m_currentRoom->historyEdge()) {
        // qWarning() << "Trying to find inexistent event:" << eventID;
        return -1;
    }
    return it - m_currentRoom->messageEvents().rbegin() + timelineBaseIndex();
}

void MessageEventModel::createEventObjects(const Quotient::RoomMessageEvent *event)
{
    auto eventId = event->id();

    if (m_linkPreviewers.contains(eventId)) {
        if (!LinkPreviewer::hasPreviewableLinks(event)) {
            m_linkPreviewers.remove(eventId);
        }
    } else {
        if (LinkPreviewer::hasPreviewableLinks(event)) {
            m_linkPreviewers[eventId] = QSharedPointer<LinkPreviewer>(new LinkPreviewer(m_currentRoom, event));
        }
    }

    // ReactionModel handles updates to add and remove reactions, we only need to
    // handle adding and removing whole models here.
    if (m_reactionModels.contains(eventId)) {
        // If a model already exists but now has no reactions remove it
        if (m_reactionModels[eventId]->rowCount() <= 0) {
            m_reactionModels.remove(eventId);
            if (!resetting) {
                refreshEventRoles(eventId, {ReactionRole, ShowReactionsRole});
            }
        }
    } else {
        if (m_currentRoom->relatedEvents(*event, Quotient::EventRelation::AnnotationType).count() > 0) {
            // If a model doesn't exist and there are reactions add it.
            auto reactionModel = QSharedPointer<ReactionModel>(new ReactionModel(event, m_currentRoom));
            if (reactionModel->rowCount() > 0) {
                m_reactionModels[eventId] = reactionModel;
                if (!resetting) {
                    refreshEventRoles(eventId, {ReactionRole, ShowReactionsRole});
                }
            }
        }
    }
}

bool MessageEventModel::event(QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange) {
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {AuthorRole, ReplyAuthor, ReadMarkersRole});
    }
    return QObject::event(event);
}

#include "moc_messageeventmodel.cpp"
