// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roomthreadsmodel.h"

#include <Quotient/csapi/threads_list.h>
#include <Quotient/jobs/basejob.h>
#include <qvariant.h>

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

    auto connection = m_room->connection();
    auto threadsJob = connection->callApi<Quotient::GetThreadRootsJob>(m_room->id());
    connect(threadsJob, &Quotient::BaseJob::success, this, [this, threadsJob]() {
        qDeleteAll(m_threadModels);
        m_threadModels.clear();

        beginResetModel();
        auto threads = threadsJob->chunk();
        for (auto &thread : threads) {
            m_threadModels.push_back(new ThreadModel(m_room, std::move(thread), this));
        }
        endResetModel();
    });
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
