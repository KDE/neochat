// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "threadmodel.h"

#include <Quotient/events/event.h>
#include <Quotient/events/stickerevent.h>
#include <Quotient/jobs/basejob.h>
#include <memory>

#include "chatbarcache.h"
#include "eventhandler.h"
#include "messagecomponenttype.h"
#include "messagecontentmodel.h"
#include "neochatroom.h"

ThreadModel::ThreadModel(const QString &threadRootId, NeoChatRoom *room)
    : QConcatenateTablesProxyModel(room)
    , m_threadRootId(threadRootId)
    , m_threadFetchModel(new ThreadFetchModel(this))
    , m_threadChatBarModel(new ThreadChatBarModel(this, room))
{
    Q_ASSERT(!m_threadRootId.isEmpty());
    Q_ASSERT(room);

#if Quotient_VERSION_MINOR > 9 || (Quotient_VERSION_MINOR == 9 && Quotient_VERSION_PATCH > 0)
    connect(room, &Quotient::Room::pendingEventAdded, this, [this](const Quotient::RoomEvent *event) {
#else
    connect(room, &Quotient::Room::pendingEventAboutToAdd, this, [this](Quotient::RoomEvent *event) {
#endif
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
    const auto room = dynamic_cast<NeoChatRoom *>(QObject::parent());
    if (room == nullptr) {
        return;
    }

    for (auto i = room->pendingEvents().rbegin(); i != room->pendingEvents().rend(); i++) {
        if (const auto roomMessageEvent = eventCast<const Quotient::RoomMessageEvent>(i->event());
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
        const auto room = dynamic_cast<NeoChatRoom *>(QObject::parent());
        const auto connection = room->connection();
        m_currentJob = connection->callApi<Quotient::GetRelatingEventsWithRelTypeJob>(room->id(), m_threadRootId, u"m.thread"_s, *m_nextBatch, QString(), max);
        Q_EMIT moreEventsAvailableChanged();
        connect(m_currentJob, &Quotient::BaseJob::success, this, [this]() {
            auto newEvents = m_currentJob->chunk();
            for (auto &event : newEvents) {
                m_events.push_back(event->id());
            }

            addModels();

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
    m_events.push_front(eventId);
}

void ThreadModel::addModels()
{
    if (!sourceModels().isEmpty()) {
        clearModels();
    }

    const auto room = dynamic_cast<NeoChatRoom *>(QObject::parent());
    if (room == nullptr) {
        return;
    }
    addSourceModel(m_threadFetchModel);
    for (auto it = m_events.crbegin(); it != m_events.crend(); ++it) {
        const auto contentModel = room->contentModelForEvent(*it);
        if (contentModel != nullptr) {
            addSourceModel(room->contentModelForEvent(*it));
        }
    }
    addSourceModel(m_threadChatBarModel);

    beginResetModel();
    endResetModel();
}

void ThreadModel::clearModels()
{
    const auto room = dynamic_cast<NeoChatRoom *>(QObject::parent());
    if (room == nullptr) {
        return;
    }
    removeSourceModel(m_threadFetchModel);
    for (const auto &model : m_events) {
        const auto contentModel = room->contentModelForEvent(model);
        if (sourceModels().contains(contentModel)) {
            removeSourceModel(contentModel);
        }
    }
    removeSourceModel(m_threadChatBarModel);
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
            const auto sourceContentModel = dynamic_cast<MessageContentModel *>(model);
            if (sourceContentModel == nullptr) {
                return;
            }
            sourceContentModel->closeLinkPreview(sourceIndex.row());
        }
    }
}

ThreadFetchModel::ThreadFetchModel(QObject *parent)
    : QAbstractListModel(parent)
{
    const auto threadModel = dynamic_cast<ThreadModel *>(parent);
    Q_ASSERT(threadModel != nullptr);
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
        return MessageComponentType::FetchButton;
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

ThreadChatBarModel::ThreadChatBarModel(QObject *parent, NeoChatRoom *room)
    : QAbstractListModel(parent)
    , m_room(room)
{
    if (m_room != nullptr) {
        connect(m_room->threadCache(), &ChatBarCache::threadIdChanged, this, [this](const QString &oldThreadId, const QString &newThreadId) {
            const auto threadModel = dynamic_cast<ThreadModel *>(this->parent());
            if (threadModel != nullptr && (oldThreadId == threadModel->threadRootId() || newThreadId == threadModel->threadRootId())) {
                beginResetModel();
                endResetModel();
            }
        });
    }
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
        return m_room->threadCache()->threadId() == threadModel->threadRootId() ? MessageComponentType::ChatBar : MessageComponentType::ReplyButton;
    }
    if (role == ChatBarCacheRole) {
        if (m_room == nullptr) {
            return {};
        }
        return QVariant::fromValue<ChatBarCache *>(m_room->threadCache());
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
        {ChatBarCacheRole, "chatBarCache"},
        {ThreadRootRole, "threadRoot"},
    };
}

#include "moc_threadmodel.cpp"
