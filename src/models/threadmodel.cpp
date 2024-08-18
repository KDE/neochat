// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "threadmodel.h"

#include <Quotient/csapi/relations.h>
#include <Quotient/events/event.h>
#include <Quotient/events/stickerevent.h>
#include <Quotient/jobs/basejob.h>
#include <Quotient/omittable.h>
#include <memory>

#include "eventhandler.h"
#include "messagecontentmodel.h"
#include "neochatroom.h"

ThreadModel::ThreadModel(const QString &threadRootId, NeoChatRoom *room)
    : QConcatenateTablesProxyModel(room)
    , m_threadRootId(threadRootId)
{
    Q_ASSERT(!m_threadRootId.isEmpty());
    Q_ASSERT(room);

    m_threadRootContentModel = std::unique_ptr<MessageContentModel>(new MessageContentModel(room, threadRootId));

    connect(room, &Quotient::Room::pendingEventAboutToAdd, this, [this](Quotient::RoomEvent *event) {
        if (auto roomEvent = eventCast<const Quotient::RoomMessageEvent>(event)) {
            EventHandler eventHandler(dynamic_cast<NeoChatRoom *>(QObject::parent()), roomEvent);
            if (eventHandler.isThreaded() && eventHandler.threadRoot() == m_threadRootId) {
                addNewEvent(event);
                clearModels();
                addModels();
            }
        }
    });
    connect(room, &Quotient::Room::aboutToAddNewMessages, this, [this](Quotient::RoomEventsRange events) {
        for (const auto &event : events) {
            if (auto roomEvent = eventCast<const Quotient::RoomMessageEvent>(event)) {
                EventHandler eventHandler(dynamic_cast<NeoChatRoom *>(QObject::parent()), roomEvent);
                if (eventHandler.isThreaded() && eventHandler.threadRoot() == m_threadRootId) {
                    addNewEvent(roomEvent);
                }
            }
        }
        clearModels();
        addModels();
    });

    fetchMore({});
    addModels();
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

            clearModels();
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
    addSourceModel(m_threadRootContentModel.get());
    for (auto it = m_contentModels.crbegin(); it != m_contentModels.crend(); ++it) {
        addSourceModel(*it);
    }

    beginResetModel();
    endResetModel();
}

void ThreadModel::clearModels()
{
    removeSourceModel(m_threadRootContentModel.get());
    for (const auto &model : m_contentModels) {
        if (sourceModels().contains(model)) {
            removeSourceModel(model);
        }
    }
}

#include "moc_threadmodel.cpp"
