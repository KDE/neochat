// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roomthreadsmodel.h"

#include <Quotient/jobs/basejob.h>
#include <Quotient/omittable.h>

#include "eventhandler.h"
#include "threadmodel.h"

RoomThreadsModel::RoomThreadsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

NeoChatRoom *RoomThreadsModel::room() const
{
    return m_room;
}

void RoomThreadsModel::setRoom(NeoChatRoom *room)
{
    if (m_threadModel == nullptr) {
        return;
    }
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
    m_threadRoots.clear();
    m_threadModel->setThreadRootEvent(nullptr);

    m_room = room;
    Q_EMIT roomChanged();

    if (m_room != nullptr) {
        m_nextBatch = QString();
    }

    endResetModel();

    if (m_room != nullptr) {
        fetchMore({});
    }
}

void RoomThreadsModel::setThreadModel(ThreadModel *threadModel)
{
    m_threadModel = threadModel;
}

QVariant RoomThreadsModel::data(const QModelIndex &index, int role) const
{
    const auto row = index.row();
    const auto event = m_threadRoots[row].get();

    EventHandler eventHandler;
    eventHandler.setRoom(m_room);
    eventHandler.setEvent(event);

    switch (role) {
    case DisplayRole:
        return eventHandler.getRichBody();
    case EventIdRole:
        return eventHandler.getId();
    case AuthorRole:
        return eventHandler.getAuthor();
    case TimeRole:
        return eventHandler.getTime();
    case TimeStringRole:
        return eventHandler.getTimeString(false);
    case ThreadModelRole:
        return QVariant::fromValue<ThreadModel *>(m_threadModel);
    }

    return {};
}

int RoomThreadsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_threadRoots.size();
}

QHash<int, QByteArray> RoomThreadsModel::roleNames() const
{
    return {
        {DisplayRole, "displayText"},
        {EventIdRole, "eventId"},
        {AuthorRole, "author"},
        {TimeRole, "time"},
        {TimeStringRole, "timeString"},
        {ThreadModelRole, "threadModel"},
    };
}

bool RoomThreadsModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return !m_currentJob && m_nextBatch.has_value();
}

void RoomThreadsModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (!m_currentJob && m_nextBatch.has_value()) {
        auto connection = m_room->connection();
        m_currentJob = connection->callApi<Quotient::GetThreadRootsJob>(m_room->id(), QString(), Quotient::none, *m_nextBatch);
        connect(m_currentJob, &Quotient::BaseJob::success, this, [this]() {
            auto newThreads = m_currentJob->chunk();
            beginInsertRows({}, rowCount(), rowCount() + newThreads.size() - 1);
            for (auto &thread : newThreads) {
                m_threadRoots.push_back(std::move(thread));
            }
            endInsertRows();

            const auto newNextBatch = m_currentJob->nextBatch();
            if (!newNextBatch.isEmpty() && *m_nextBatch != newNextBatch) {
                *m_nextBatch = newNextBatch;
            } else {
                m_nextBatch.reset();
            }

            m_currentJob.clear();
        });
    }
}

void RoomThreadsModel::selectThread(const QString &threadRootId)
{
    if (threadRootId.isEmpty()) {
        return;
    }
    for (const auto &threadRoot : m_threadRoots) {
        if (threadRoot->id() == threadRootId) {
            m_threadModel->setThreadRootEvent(threadRoot.get());
        }
    }
}
