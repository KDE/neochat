// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "models/limitermodel.h"

LimiterModel::LimiterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &QSortFilterProxyModel::rowsInserted, this, &LimiterModel::extraCountChanged);
    connect(this, &QSortFilterProxyModel::rowsRemoved, this, &LimiterModel::extraCountChanged);
    connect(this, &QSortFilterProxyModel::modelReset, this, &LimiterModel::extraCountChanged);
}

int LimiterModel::maximumCount() const
{
    return m_maximumCount;
}

void LimiterModel::setMaximumCount(int maximumCount)
{
    if (m_maximumCount != maximumCount) {
        m_maximumCount = maximumCount;
        Q_EMIT maximumCountChanged();
    }
}

int LimiterModel::extraCount() const
{
    if (sourceModel()) {
        return std::max(sourceModel()->rowCount() - maximumCount(), 0);
    }
    return 0;
}

bool LimiterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)
    return source_row < maximumCount();
}

#include "moc_limitermodel.cpp"
