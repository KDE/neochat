// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "threadmodel.h"

#include <Quotient/csapi/relations.h>
#include <Quotient/events/event.h>
#include <Quotient/events/stickerevent.h>
#include <Quotient/jobs/basejob.h>

#include "eventhandler.h"
#include "neochatroom.h"
#include "reactionmodel.h"
#include "roomthreadsmodel.h"

ThreadModel::ThreadModel(QObject *parent)
    : QAbstractListModel(parent)
{
    if (auto roomThreadsModel = dynamic_cast<RoomThreadsModel *>(this->parent())) {
        roomThreadsModel->setThreadModel(this);
    }
}

NeoChatRoom *ThreadModel::room() const
{
    return m_room;
}

void ThreadModel::setRoom(NeoChatRoom *room)
{
    if (room == m_room) {
        return;
    }

    beginResetModel();
    if (!m_currentJob.isNull()) {
        if (m_currentJob->status() == Quotient::BaseJob::Pending) {
            m_currentJob->abandon();
        }
        m_currentJob.clear();
    }
    m_nextBatch->clear();
    m_threadRootEvent = nullptr;
    m_events.clear();
    m_pendingEvents.clear();
    m_linkPreviewers.clear();
    m_reactionModels.clear();
    m_addingPending = false;
    m_loading = false;
    Q_EMIT loadingChanged();

    if (m_room) {
        m_room->disconnect(this);
    }

    m_room = room;
    Q_EMIT roomChanged();

    if (m_room) {
        connect(m_room, &Quotient::Room::pendingEventAboutToAdd, this, [this](Quotient::RoomEvent *event) {
            if (auto roomEvent = eventCast<const Quotient::RoomMessageEvent>(event)) {
                EventHandler eventHandler;
                eventHandler.setRoom(m_room);
                eventHandler.setEvent(roomEvent);
                if (eventHandler.isThreaded() && eventHandler.threadRoot() == m_threadRootId) {
                    beginInsertRows({}, 0, 0);
                    m_pendingEvents.push_front(Quotient::loadEvent<Quotient::RoomEvent>(event->fullJson()));
                    m_addingPending = true;
                }
            }
        });
        connect(m_room, &Quotient::Room::pendingEventAdded, this, [this]() {
            if (m_addingPending) {
                endInsertRows();
                m_addingPending = false;
            }
        });
        connect(m_room, &Quotient::Room::pendingEventAboutToMerge, this, [this](Quotient::RoomEvent *serverEvent) {
            EventHandler eventHandler;
            eventHandler.setRoom(m_room);
            eventHandler.setEvent(serverEvent);
            if (eventHandler.isThreaded() && eventHandler.threadRoot() == m_threadRootId) {
                addNewEvent(serverEvent);
            }
        });
        connect(this, &ThreadModel::rowsInserted, this, [this]() {
            if (m_loading) {
                m_loading = false;
                Q_EMIT loadingChanged();
            }
        });
    }

    endResetModel();
}

void ThreadModel::setThreadRootEvent(Quotient::RoomEvent *threadRootEvent)
{
    if (m_room == nullptr) {
        return;
    }
    if (threadRootEvent == m_threadRootEvent) {
        return;
    }

    m_threadRootEvent = std::move(threadRootEvent);

    beginResetModel();

    if (m_threadRootEvent != nullptr) {
        m_threadRootId = m_threadRootEvent->id();
        m_currentJob.clear();
        m_events.clear();
        m_pendingEvents.clear();
        m_linkPreviewers.clear();
        m_reactionModels.clear();
        m_nextBatch = QString();
        m_addingPending = false;
        m_loading = false;
        Q_EMIT loadingChanged();
    }

    endResetModel();
}

QString ThreadModel::threadRootId() const
{
    return m_threadRootId;
}

QVariantMap ThreadModel::threadRootAuthor() const
{
    EventHandler eventHandler;
    eventHandler.setRoom(m_room);
    eventHandler.setEvent(m_threadRootEvent);

    return eventHandler.getAuthor();
}

QDateTime ThreadModel::threadRootTime() const
{
    EventHandler eventHandler;
    eventHandler.setRoom(m_room);
    eventHandler.setEvent(m_threadRootEvent);

    return eventHandler.getTime();
}

QString ThreadModel::threadRootTimeString() const
{
    EventHandler eventHandler;
    eventHandler.setRoom(m_room);
    eventHandler.setEvent(m_threadRootEvent);

    return eventHandler.getTimeString(false);
}

QString ThreadModel::threadRootDisplay() const
{
    EventHandler eventHandler;
    eventHandler.setRoom(m_room);
    eventHandler.setEvent(m_threadRootEvent);

    return eventHandler.getRichBody();
}

bool ThreadModel::loading() const
{
    return m_loading;
}

static LinkPreviewer *emptyLinkPreview = new LinkPreviewer;

QVariant ThreadModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto row = index.row();
    if (row < 0 || row >= rowCount()) {
        return {};
    }
    const bool isPending = row < int(m_pendingEvents.size());

    Quotient::RoomEvent *event = nullptr;
    if (row == rowCount() - 1 && !m_nextBatch.has_value()) {
        event = m_threadRootEvent;
    } else {
        event = isPending ? m_pendingEvents[row].get() : m_events[row - m_pendingEvents.size()].get();
    }

    EventHandler eventHandler;
    eventHandler.setRoom(m_room);
    eventHandler.setEvent(event);

    switch (role) {
    case DisplayRole:
        return eventHandler.getRichBody();
    case PlainTextRole:
        return eventHandler.getPlainBody();
    case GenericDisplayRole:
        return eventHandler.getGenericBody();
    case EventIdRole:
        return eventHandler.getId();
    case DelegateTypeRole:
        return eventHandler.getDelegateType();
    case AuthorRole:
        return eventHandler.getAuthor();
    case ShowAuthorRole:
        for (auto r = row + 1; r < rowCount(); ++r) {
            auto i = this->index(r);
            // Note !itemData(i).empty() is a check for instances where rows have been removed, e.g. when the read marker is moved.
            // While the row is removed the subsequent row indexes are not changed so we need to skip over the removed index.
            // See - https://doc.qt.io/qt-5/qabstractitemmodel.html#beginRemoveRows
            if (data(i, SpecialMarksRole) != Quotient::EventStatus::Hidden && !itemData(i).empty()) {
                return data(i, AuthorRole) != data(index, AuthorRole) || data(i, DelegateTypeRole) == DelegateType::State
                    || data(i, TimeRole).toDateTime().msecsTo(data(index, TimeRole).toDateTime()) > 600000
                    || data(i, TimeRole).toDateTime().toLocalTime().date().day() != data(index, TimeRole).toDateTime().toLocalTime().date().day();
            }
        }

        return true;
    case TimeRole:
        return eventHandler.getTime();
    case TimeStringRole:
        return eventHandler.getTimeString(false);
    case SectionRole:
        return eventHandler.getTimeString(true);
    case ShowSectionRole:
        for (auto r = row + 1; r < rowCount(); ++r) {
            auto i = this->index(r);
            // Note !itemData(i).empty() is a check for instances where rows have been removed, e.g. when the read marker is moved.
            // While the row is removed the subsequent row indexes are not changed so we need to skip over the removed index.
            // See - https://doc.qt.io/qt-5/qabstractitemmodel.html#beginRemoveRows
            if (data(i, SpecialMarksRole) != Quotient::EventStatus::Hidden && !itemData(i).empty()) {
                const auto day = data(index, TimeRole).toDateTime().toLocalTime().date().dayOfYear();
                const auto previousEventDay = data(i, TimeRole).toDateTime().toLocalTime().date().dayOfYear();
                return day != previousEventDay;
            }
        }

        return false;
    case HighlightRole:
        return eventHandler.isHighlighted();
    case SpecialMarksRole:
        if (eventHandler.isHidden()) {
            return Quotient::EventStatus::Hidden;
        }
        return Quotient::EventStatus::Normal;
    case MediaInfoRole:
        return eventHandler.getMediaInfo();
    case LinkPreviewRole:
        if (m_linkPreviewers.contains(event->id())) {
            return QVariant::fromValue<LinkPreviewer *>(m_linkPreviewers[event->id()].data());
        } else {
            return QVariant::fromValue<LinkPreviewer *>(emptyLinkPreview);
        }
    case ShowLinkPreviewRole:
        return m_linkPreviewers.contains(event->id());
    case ReactionRole:
        if (m_reactionModels.contains(event->id())) {
            return QVariant::fromValue<ReactionModel *>(m_reactionModels[event->id()].data());
        } else {
            return QVariantList();
        }
    case ShowReactionsRole:
        return m_reactionModels.contains(event->id());
    case IsReplyRole:
        return eventHandler.hasReply(false);
    case ReplyIdRole:
        return eventHandler.getReplyId();
    case ReplyDelegateTypeRole:
        return eventHandler.getReplyDelegateType();
    case ReplyAuthorRole:
        return eventHandler.getReplyAuthor();
    case ReplyDisplayRole:
        return eventHandler.getReplyRichBody();
    case ReplyMediaInfoRole:
        return eventHandler.getReplyMediaInfo();
    case IsThreadedRole:
        return eventHandler.isThreaded();
    case ThreadRootRole:
        return eventHandler.threadRoot();
    case LatitudeRole:
        return eventHandler.getLatitude();
    case LongitudeRole:
        return eventHandler.getLongitude();
    case AssetRole:
        return eventHandler.getLocationAssetType();
    case ShowReadMarkersRole:
        return false;
    case ExcessReadMarkersRole:
        return QString(); // To stop spam in the console.
    case ProgressInfoRole:
        if (auto e = eventCast<const Quotient::RoomMessageEvent>(event)) {
            if (e->hasFileContent()) {
                return QVariant::fromValue(m_room->fileTransferInfo(e->id()));
            }
        }
        if (auto e = eventCast<const Quotient::StickerEvent>(event)) {
            return QVariant::fromValue(m_room->fileTransferInfo(e->id()));
        }
        break;
    case IsPendingRole:
        return false;
    }
    return {};
}

int ThreadModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    // We only add the extra one for the thread root if all other messages have been
    // loaded.
    return int(m_events.size()) + int(m_pendingEvents.size()) + (m_nextBatch.has_value() ? 0 : 1);
}

QHash<int, QByteArray> ThreadModel::roleNames() const
{
    return {
        {DisplayRole, "display"},
        {PlainTextRole, "plainText"},
        {GenericDisplayRole, "genericDisplay"},
        {EventIdRole, "eventId"},
        {DelegateTypeRole, "delegateType"},
        {AuthorRole, "author"},
        {ShowAuthorRole, "showAuthor"},
        {TimeRole, "time"},
        {TimeStringRole, "timeString"},
        {SectionRole, "section"},
        {ShowSectionRole, "showSection"},
        {HighlightRole, "isHighlighted"},
        {SpecialMarksRole, "marks"},
        {MediaInfoRole, "mediaInfo"},
        {LinkPreviewRole, "linkPreview"},
        {ShowLinkPreviewRole, "showLinkPreview"},
        {ReactionRole, "reaction"},
        {ShowReactionsRole, "showReactions"},
        {IsReplyRole, "isReply"},
        {ReplyIdRole, "replyId"},
        {ReplyDelegateTypeRole, "replyDelegateType"},
        {ReplyAuthorRole, "replyAuthor"},
        {ReplyDisplayRole, "replyDisplay"},
        {ReplyMediaInfoRole, "replyMediaInfo"},
        {IsThreadedRole, "isThreaded"},
        {ThreadRootRole, "threadRoot"},
        {LatitudeRole, "latitude"},
        {LongitudeRole, "longitude"},
        {AssetRole, "asset"},
        {ReadMarkersRole, "readMarkers"},
        {ExcessReadMarkersRole, "excessReadMarkers"},
        {ReadMarkersString, "readMarkersString"},
        {ShowReadMarkersRole, "showReadMarkers"},
        {ProgressInfoRole, "progressInfo"},
        {VerifiedRole, "verified"},
        {IsPendingRole, "isPending"},
    };
}

bool ThreadModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return !m_currentJob && m_nextBatch.has_value();
}

void ThreadModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (!m_currentJob && m_nextBatch.has_value()) {
        auto connection = m_room->connection();
        m_currentJob = connection->callApi<Quotient::GetRelatingEventsWithRelTypeJob>(m_room->id(), m_threadRootId, QLatin1String("m.thread"), *m_nextBatch);
        connect(m_currentJob, &Quotient::BaseJob::success, this, [this]() {
            auto newEvents = m_currentJob->chunk();
            beginInsertRows(QModelIndex(), rowCount(), rowCount() + newEvents.size() - 1);
            for (auto &event : newEvents) {
                auto messageEvent = Quotient::eventCast<Quotient::RoomMessageEvent>(event);
                createEventObjects(messageEvent);
                m_events.push_back(std::move(event));
            }
            endInsertRows();

            const auto newNextBatch = m_currentJob->nextBatch();
            if (!newNextBatch.isEmpty() && *m_nextBatch != newNextBatch) {
                *m_nextBatch = newNextBatch;
            } else {
                // Insert the thread root at the end.
                beginInsertRows({}, rowCount(), rowCount());
                endInsertRows();
                m_nextBatch.reset();
            }

            m_currentJob.clear();
        });
    }
}

void ThreadModel::addNewEvent(const Quotient::RoomEvent *event)
{
    for (auto it = m_pendingEvents.begin(); it != m_pendingEvents.end();) {
        const auto pendingEvent = it->get();
        if (pendingEvent->transactionId() == event->transactionId()) {
            int startRow = std::min(int(it - m_pendingEvents.begin()), 0);
            int endRow = std::min(int(m_pendingEvents.size()) - 1, 0);
            if (startRow != endRow) {
                beginMoveRows({}, startRow, startRow, {}, endRow);
            }
            m_pendingEvents.erase(it);
            m_events.push_front(Quotient::loadEvent<Quotient::RoomEvent>(event->fullJson()));
            if (startRow == endRow) {
                dataChanged({}, index(startRow, startRow));
            } else {
                endMoveRows();
            }
            return;
        } else {
            ++it;
        }
    }
}

void ThreadModel::createEventObjects(const Quotient::RoomMessageEvent *event)
{
    if (event == nullptr) {
        return;
    }

    auto eventId = event->id();

    EventHandler eventHandler;
    eventHandler.setRoom(m_room);
    eventHandler.setEvent(event);

    if (auto linkPreviewer = eventHandler.getLinkPreviewer()) {
        m_linkPreviewers[eventId] = linkPreviewer;
    } else {
        m_linkPreviewers.remove(eventId);
    }
    if (auto reactionModel = eventHandler.getReactions()) {
        m_reactionModels[eventId] = reactionModel;
    } else {
        m_reactionModels.remove(eventId);
    }
}
#include "moc_threadmodel.cpp"
