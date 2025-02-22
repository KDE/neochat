// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "commonroomsmodel.h"
#include "jobs/neochatgetcommonroomsjob.h"

#include <QGuiApplication>

using namespace Quotient;

CommonRoomsModel::CommonRoomsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

NeoChatConnection *CommonRoomsModel::connection() const
{
    return m_connection;
}

void CommonRoomsModel::setConnection(NeoChatConnection *connection)
{
    m_connection = connection;
    Q_EMIT connectionChanged();
    reload();
}

QString CommonRoomsModel::userId() const
{
    return m_userId;
}

void CommonRoomsModel::setUserId(const QString &userId)
{
    m_userId = userId;
    Q_EMIT userIdChanged();
    reload();
}

QVariant CommonRoomsModel::data(const QModelIndex &index, int roleName) const
{
    Q_UNUSED(index)
    Q_UNUSED(roleName)
    return {};
}

int CommonRoomsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_commonRooms.size();
}

void CommonRoomsModel::reload()
{
    if (!m_connection || m_userId.isEmpty()) {
        return;
    }

    if (!m_connection->canCheckMutualRooms()) {
        return;
    }

    // Checking if you have mutual rooms with yourself doesn't make sense and servers reject it too
    if (m_connection->userId() == m_userId) {
        return;
    }

    m_connection->callApi<NeochatGetCommonRoomsJob>(m_userId).then([this](const auto job) {
        const auto &replyData = job->jsonData();
        beginResetModel();
        for (const auto &roomId : replyData[u"joined"_s].toArray()) {
            m_commonRooms.push_back(roomId.toString());
        }
        endResetModel();
        Q_EMIT countChanged();
    });
}

#include "moc_commonroomsmodel.cpp"
