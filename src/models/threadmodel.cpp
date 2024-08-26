// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "threadmodel.h"

#include <Quotient/events/event.h>
#include <Quotient/events/stickerevent.h>
#include <Quotient/jobs/basejob.h>
#include <Quotient/omittable.h>
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

    connect(room, &Quotient::Room::pendingEventAboutToAdd, this, [this](Quotient::RoomEvent *event) {
        if (auto roomEvent = eventCast<const Quotient::RoomMessageEvent>(event)) {
            if (EventHandler::isThreaded(roomEvent) && EventHandler::threadRoot(roomEvent) == m_threadRootId) {
                addNewEvent(event);
                addModels();
            }
        }
    });
    connect(room, &Quotient::Room::aboutToAddNewMessages, this, [this](Quotient::RoomEventsRange events) {
        for (const auto &event : events) {
            if (auto roomEvent = eventCast<const Quotient::RoomMessageEvent>(event)) {
                if (EventHandler::isThreaded(roomEvent) && EventHandler::threadRoot(roomEvent) == m_threadRootId) {
                    addNewEvent(roomEvent);
                }
            }
        }
        addModels();
    });

    fetchMore({});
    addModels();
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
        m_currentJob =
            connection->callApi<Quotient::GetRelatingEventsWithRelTypeJob>(room->id(), m_threadRootId, QLatin1String("m.thread"), *m_nextBatch, QString(), 5);
        connect(m_currentJob, &Quotient::BaseJob::success, this, [this]() {
            const auto room = dynamic_cast<NeoChatRoom *>(QObject::parent());
            auto newEvents = m_currentJob->chunk();
            for (auto &event : newEvents) {
                m_contentModels.push_back(new MessageContentModel(room, event.get()));
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
    const auto room = dynamic_cast<NeoChatRoom *>(QObject::parent());
    m_contentModels.push_front(new MessageContentModel(room, event));
}

void ThreadModel::addModels()
{
    if (!sourceModels().isEmpty()) {
        clearModels();
    }

    addSourceModel(m_threadRootContentModel.get());
    for (auto it = m_contentModels.crbegin(); it != m_contentModels.crend(); ++it) {
        addSourceModel(*it);
    }
    addSourceModel(m_threadChatBarModel);
}

void ThreadModel::clearModels()
{
    removeSourceModel(m_threadRootContentModel.get());
    for (const auto &model : m_contentModels) {
        if (sourceModels().contains(model)) {
            removeSourceModel(model);
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

    if (role == ComponentTypeRole) {
        return MessageComponentType::ChatBar;
    }
    if (role == ChatBarCacheRole) {
        if (m_room == nullptr) {
            return {};
        }
        return QVariant::fromValue<ChatBarCache *>(m_room->threadCache());
    }
    return {};
}

int ThreadChatBarModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (m_room == nullptr) {
        return 0;
    }
    const auto threadModel = dynamic_cast<ThreadModel *>(this->parent());
    if (threadModel != nullptr) {
        return m_room->threadCache()->threadId() == threadModel->threadRootId() ? 1 : 0;
    }
    return 0;
}

QHash<int, QByteArray> ThreadChatBarModel::roleNames() const
{
    return {
        {ComponentTypeRole, "componentType"},
        {ChatBarCacheRole, "chatBarCache"},
    };
}

#include "moc_threadmodel.cpp"
