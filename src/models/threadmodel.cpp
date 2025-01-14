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
#include "neochatroom.h"

ThreadModel::ThreadModel(const QString &threadRootId, NeoChatRoom *room)
    : QConcatenateTablesProxyModel(room)
    , m_threadRootId(threadRootId)
    , m_threadChatBarModel(new ThreadChatBarModel(this, room))
{
    Q_ASSERT(!m_threadRootId.isEmpty());
    Q_ASSERT(room);

    m_threadRootContentModel = std::unique_ptr<MessageContentModel>(new MessageContentModel(room, threadRootId));

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
    fetchMore({});
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

MessageContentModel *ThreadModel::threadRootContentModel() const
{
    return m_threadRootContentModel.get();
}

QHash<int, QByteArray> ThreadModel::roleNames() const
{
    return m_threadRootContentModel->roleNames();
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
        const auto room = dynamic_cast<NeoChatRoom *>(QObject::parent());
        const auto connection = room->connection();
        m_currentJob = connection->callApi<Quotient::GetRelatingEventsWithRelTypeJob>(room->id(), m_threadRootId, u"m.thread"_s, *m_nextBatch, QString(), 5);
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

    addSourceModel(m_threadRootContentModel.get());
    const auto room = dynamic_cast<NeoChatRoom *>(QObject::parent());
    if (room == nullptr) {
        return;
    }
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
    removeSourceModel(m_threadRootContentModel.get());

    const auto room = dynamic_cast<NeoChatRoom *>(QObject::parent());
    if (room == nullptr) {
        return;
    }
    for (const auto &model : m_events) {
        const auto contentModel = room->contentModelForEvent(model);
        if (sourceModels().contains(contentModel)) {
            removeSourceModel(contentModel);
        }
    }
    removeSourceModel(m_threadChatBarModel);
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
