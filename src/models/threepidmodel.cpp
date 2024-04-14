// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "threepidmodel.h"

#include "neochatconnection.h"

ThreePIdModel::ThreePIdModel(NeoChatConnection *connection)
    : QAbstractListModel(connection)
{
    Q_ASSERT(connection);
    connect(connection, &NeoChatConnection::stateChanged, this, [this]() {
        const auto connection = dynamic_cast<NeoChatConnection *>(this->parent());
        if (connection != nullptr && connection->isLoggedIn()) {
            const auto threePIdJob = connection->callApi<Quotient::GetAccount3PIDsJob>();
            connect(threePIdJob, &Quotient::BaseJob::success, this, [this, threePIdJob]() {
                beginResetModel();
                m_threePIds = threePIdJob->threepids();
                endResetModel();
            });
        }
    });
}

QVariant ThreePIdModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.row() >= rowCount()) {
        qDebug() << "ThreePIdModel, something's wrong: index.row() >= m_threePIds.count()";
        return {};
    }

    if (role == AddressRole) {
        return m_threePIds.at(index.row()).address;
    }
    if (role == MediumRole) {
        return m_threePIds.at(index.row()).medium;
    }

    return {};
}

int ThreePIdModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_threePIds.count();
}

QHash<int, QByteArray> ThreePIdModel::roleNames() const
{
    return {
        {AddressRole, QByteArrayLiteral("address")},
        {MediumRole, QByteArrayLiteral("medium")},
    };
}
