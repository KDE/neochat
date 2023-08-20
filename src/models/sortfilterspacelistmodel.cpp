// SPDX-FileCopyrightText: 2022 Snehit Sah <hi@snehit.dev>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "sortfilterspacelistmodel.h"

#include "roomlistmodel.h"

SortFilterSpaceListModel::SortFilterSpaceListModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{
    setSortRole(RoomListModel::RoomIdRole);
    sort(0);
    invalidateFilter();
    connect(this, &QAbstractProxyModel::sourceModelChanged, this, [this]() {
        connect(sourceModel(), &QAbstractListModel::dataChanged, this, [this](const QModelIndex &, const QModelIndex &, QVector<int> roles) {
            if (roles.contains(RoomListModel::IsChildSpaceRole)) {
                invalidate();
            }
            countChanged();
        });
        invalidate();
        Q_EMIT countChanged();
    });
}

bool SortFilterSpaceListModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);
    return sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::IsSpaceRole).toBool()
        && sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::JoinStateRole).toString() != QStringLiteral("upgraded")
        && !sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::IsChildSpaceRole).toBool();
}

bool SortFilterSpaceListModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const auto idLeft = sourceModel()->data(source_left, RoomListModel::RoomIdRole).toString();
    const auto idRight = sourceModel()->data(source_right, RoomListModel::RoomIdRole).toString();
    return idLeft < idRight;
}

#include "moc_sortfilterspacelistmodel.cpp"
