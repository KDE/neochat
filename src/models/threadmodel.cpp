// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "threadmodel.h"

#include <Quotient/csapi/relations.h>
#include <Quotient/events/stickerevent.h>
#include <Quotient/jobs/basejob.h>

#include "eventhandler.h"
#include "neochatroom.h"
#include "reactionmodel.h"

ThreadModel::ThreadModel(const NeoChatRoom *room, const QString &threadRootId, QObject *parent)
    : QAbstractListModel(parent)
    , m_room(room)
    , m_threadRootId(threadRootId)
{
    connect(this, &ThreadModel::rowsInserted, this, [this]() {
        if (m_loading) {
            m_loading = false;
            Q_EMIT loadingChanged();
        }
    });
    intializeModel();
}

QString ThreadModel::threadRootId() const
{
    return m_threadRootId;
}

bool ThreadModel::loading() const
{
    return m_loading;
}

void ThreadModel::intializeModel()
{
    if (m_threadRootId.isEmpty() || m_room == nullptr) {
        return;
    }
    m_events.clear();
    *m_nextBatch = QString();
    m_loading = true;
}

static LinkPreviewer *emptyLinkPreview = new LinkPreviewer;

QVariant ThreadModel::data(const QModelIndex &index, int role) const
{
    const auto row = index.row();
    const auto event = m_events[row].get();

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
    return m_events.size();
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
        auto threadEventsJob =
            connection->callApi<Quotient::GetRelatingEventsWithRelTypeJob>(m_room->id(), m_threadRootId, QLatin1String("m.thread"), *m_nextBatch);
        connect(threadEventsJob, &Quotient::BaseJob::success, this, [this, threadEventsJob]() {
            auto newEvents = threadEventsJob->chunk();
            beginInsertRows(QModelIndex(), rowCount(), rowCount() + newEvents.size() - 1);
            for (auto &event : newEvents) {
                auto messageEvent = Quotient::eventCast<Quotient::RoomMessageEvent>(event);
                createEventObjects(messageEvent);
                m_events.emplace_back(std::move(event));
            }
            endInsertRows();

            const auto newNextBatch = threadEventsJob->nextBatch();
            if (!newNextBatch.isEmpty() && *m_nextBatch != newNextBatch) {
                *m_nextBatch = newNextBatch;
            } else {
                m_nextBatch.reset();
            }

            m_currentJob.clear();
        });
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
