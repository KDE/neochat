// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roomthreadsmodel.h"

#include <Quotient/jobs/basejob.h>
#include <Quotient/omittable.h>

#include "threadmodel.h"

RoomThreadsModel::RoomThreadsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void RoomThreadsModel::initializeModel()
{
    if (m_room == nullptr) {
        return;
    }

    if (!m_currentJob.isNull()) {
        if (m_currentJob->status() == Quotient::BaseJob::Pending) {
            m_currentJob->abandon();
        }
        m_currentJob.clear();
    }

    beginResetModel();
    qDeleteAll(m_threadModels);
    m_threadModels.clear();
    *m_nextBatch = QString();
    endResetModel();

    fetchMore({});
}

NeoChatRoom *RoomThreadsModel::room() const
{
    return m_room;
}

void RoomThreadsModel::setRoom(NeoChatRoom *room)
{
    if (room == m_room) {
        return;
    }
    m_room = room;
    Q_EMIT roomChanged();

    initializeModel();
}

QVariant RoomThreadsModel::data(const QModelIndex &index, int role) const
{
    const auto row = index.row();
    const auto model = m_threadModels[row];

    switch (role) {
    case DisplayRole:
        return model->threadRootDisplay();
    case EventIdRole:
        return model->threadRootId();
    case AuthorRole:
        return model->threadRootAuthor();
    case TimeRole:
        return model->threadRootTime();
    case TimeStringRole:
        return model->threadRootTimeString();
    case ThreadModelRole:
        return QVariant::fromValue<ThreadModel *>(model);
    }

    return {};
}

int RoomThreadsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_threadModels.size();
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
                m_threadModels.push_back(new ThreadModel(m_room, std::move(thread), this));
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
