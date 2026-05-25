// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "threadmodel.h"

#include <ranges>

#include <QTimer>

#include <Quotient/events/event.h>
#include <Quotient/events/stickerevent.h>
#include <Quotient/jobs/basejob.h>

#include "block.h"
#include "chatbarcache.h"
#include "contentprovider.h"
#include "enums/blocktype.h"
#include "eventhandler.h"
#include "eventmessagecontentmodel.h"
#include "neochatroom.h"

ThreadModel::ThreadModel(const QString &threadRootId, NeoChatRoom *room)
    : QConcatenateTablesProxyModel()
    , m_room(room)
    , m_threadRootId(threadRootId)
    , m_threadFetchModel(new ThreadFetchModel(this))
    , m_threadChatBarModel(new ThreadChatBarModel(this))
{
    Q_ASSERT(!m_threadRootId.isEmpty());
    Q_ASSERT(room);

    // HACK: Always keep at least one source model in the concatenate model to work around assert
    addSourceModel(m_threadFetchModel);

    connect(room, &Quotient::Room::pendingEventAboutToMerge, this, [this](Quotient::RoomEvent *event) {
        if (auto roomEvent = eventCast<const Quotient::RoomMessageEvent>(event)) {
            if (roomEvent->isThreaded() && roomEvent->threadRootEventId() == m_threadRootId) {
                addNewEvent(event);
                addModels();
            }
        }
    });
    connect(room, &Quotient::Room::aboutToAddNewMessages, this, [this](Quotient::RoomEventsRange events) {
        for (const auto &event : events) {
            if (auto roomEvent = eventCast<const Quotient::RoomMessageEvent>(event)) {
                if (roomEvent->isThreaded() && roomEvent->threadRootEventId() == m_threadRootId) {
                    addNewEvent(roomEvent);
                }
            }
        }
        addModels();
    });

    // If the thread was created by the local user fetchMore() won't find the current
    // pending event.
    checkPending();
    fetchMoreEvents(3);
    addModels();
}

void ThreadModel::checkPending()
{
    for (const auto &event : std::ranges::reverse_view(m_room->pendingEvents())) {
        if (const auto &roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(event.event());
            roomMessageEvent->isThreaded() && roomMessageEvent->threadRootEventId() == m_threadRootId) {
            addNewEvent(roomMessageEvent);
        }
    }
}

QString ThreadModel::threadRootId() const
{
    return m_threadRootId;
}

QHash<int, QByteArray> ThreadModel::roleNames() const
{
    return MessageContentModel::roleNamesStatic();
}

bool ThreadModel::moreEventsAvailable(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return !m_currentJob && m_nextBatch.has_value();
}

void ThreadModel::fetchMoreEvents(int max)
{
    if (!m_currentJob && m_nextBatch.has_value()) {
        m_currentJob =
            m_room->connection()->callApi<Quotient::GetRelatingEventsWithRelTypeJob>(m_room->id(), m_threadRootId, u"m.thread"_s, *m_nextBatch, QString(), max);
        Q_EMIT moreEventsAvailableChanged();
        connect(m_currentJob, &Quotient::BaseJob::success, this, [this]() {
            auto newEvents = m_currentJob->chunk();
            for (const auto &event : newEvents) {
                if (std::ranges::find(m_events, event->id()) == m_events.end()) {
                    m_events.push_back(event->id());
                }
            }

            addModels();

            const auto newNextBatch = m_currentJob->nextBatch();
            if (!newNextBatch.isEmpty() && *m_nextBatch != newNextBatch) {
                *m_nextBatch = newNextBatch;
            } else {
                m_nextBatch.reset();
            }

            m_currentJob.clear();
            Q_EMIT moreEventsAvailableChanged();
        });
    }
}

void ThreadModel::addNewEvent(const Quotient::RoomEvent *event)
{
    auto eventId = event->id();
    if (eventId.isEmpty()) {
        eventId = event->transactionId();
    }
    if (std::ranges::find(m_events, eventId) == m_events.end()) {
        m_events.push_front(eventId);
    }
}

void ThreadModel::addModels()
{
    clearModels();

    for (const auto &event : std::ranges::reverse_view(m_events)) {
        if (const auto contentModel = ContentProvider::self().contentModelForEvent(m_room, event)) {
            addSourceModel(contentModel);
        }
    }
    addSourceModel(m_threadChatBarModel);

    QTimer::singleShot(0, this, [this]() {
        m_threadChatBarModel->reset();
    });
}

void ThreadModel::clearModels()
{
    for (const auto &model : m_events) {
        const auto contentModel = ContentProvider::self().contentModelForEvent(m_room, model);
        if (sourceModels().contains(contentModel)) {
            removeSourceModel(contentModel);
        }
    }
    if (sourceModels().contains(m_threadChatBarModel)) {
        removeSourceModel(m_threadChatBarModel);
    }
}

void ThreadModel::closeLinkPreview(int row)
{
    if (row < 0 || row >= rowCount()) {
        return;
    }

    const auto index = this->index(row, 0);
    if (!index.isValid()) {
        return;
    }

    const auto sourceIndex = mapToSource(index);
    const auto sourceModel = sourceIndex.model();
    if (sourceModel == nullptr) {
        return;
    }
    // This is a bit silly but we can only get a const reference to the model from the
    // index so we need to search the source models.
    for (const auto &model : sourceModels()) {
        if (model == sourceModel) {
            const auto sourceContentModel = dynamic_cast<EventMessageContentModel *>(model);
            if (sourceContentModel == nullptr) {
                return;
            }
            sourceContentModel->closeLinkPreview(sourceIndex.row());
        }
    }
}

void ThreadModel::setReplying(bool replying)
{
    m_threadChatBarModel->setReplying(replying);
}

ThreadFetchModel::ThreadFetchModel(ThreadModel *threadModel)
    : QAbstractListModel(threadModel)
{
    Q_ASSERT(threadModel);
    connect(threadModel, &ThreadModel::moreEventsAvailableChanged, this, [this]() {
        beginResetModel();
        endResetModel();
    });
}

QVariant ThreadFetchModel::data(const QModelIndex &idx, int role) const
{
    if (idx.row() < 0 || idx.row() > 1) {
        return {};
    }

    if (role == ComponentTypeRole) {
        return Blocks::FetchButton;
    }
    return {};
}

int ThreadFetchModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    const auto threadModel = dynamic_cast<ThreadModel *>(this->parent());
    if (threadModel == nullptr) {
        qWarning() << "ThreadFetchModel created with incorrect parent, a ThreadModel must be set as the parent on creation.";
        return {};
    }
    return threadModel->moreEventsAvailable({}) ? 1 : 0;
}

QHash<int, QByteArray> ThreadFetchModel::roleNames() const
{
    return {
        {ComponentTypeRole, "componentType"},
    };
}

ThreadChatBarModel::ThreadChatBarModel(ThreadModel *threadModel)
    : QAbstractListModel(threadModel)
{
    Q_ASSERT(threadModel);
    m_block = new Blocks::Block(Blocks::ReplyButton, this);
}

QVariant ThreadChatBarModel::data(const QModelIndex &idx, int role) const
{
    if (idx.row() > 1) {
        return {};
    }

    const auto threadModel = dynamic_cast<ThreadModel *>(parent());
    if (threadModel == nullptr) {
        qWarning() << "ThreadChatBarModel created with incorrect parent, a ThreadModel must be set as the parent on creation.";
        return {};
    }
    if (role == ComponentTypeRole) {
        return m_replying ? Blocks::ChatBar : Blocks::ReplyButton;
    }
    if (role == BlockRole) {
        return m_block->toVariant();
    }
    if (role == ThreadRootRole) {
        return threadModel->threadRootId();
    }
    return {};
}

int ThreadChatBarModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QHash<int, QByteArray> ThreadChatBarModel::roleNames() const
{
    return {
        {ComponentTypeRole, "componentType"},
        {BlockRole, "block"},
        {ThreadRootRole, "threadRoot"},
    };
}

void ThreadChatBarModel::setReplying(bool replying)
{
    if (replying == m_replying) {
        return;
    }
    m_replying = replying;
    m_block->deleteLater();
    m_block = nullptr;
    if (m_replying) {
        m_block = new Blocks::ChatBarBlock(Blocks::ChatBar, false, dynamic_cast<ThreadModel *>(parent())->threadRootId(), this);
    } else {
        m_block = new Blocks::Block(Blocks::ReplyButton, this);
    }
    reset();
}

void ThreadChatBarModel::reset()
{
    beginResetModel();
    endResetModel();
}

#include "moc_threadmodel.cpp"
