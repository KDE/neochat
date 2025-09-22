// SPDX-FileCopyrightText: 2022 Snehit Sah <hi@snehit.dev>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "sortfilterspacelistmodel.h"

using namespace Qt::StringLiterals;

SortFilterSpaceListModel::SortFilterSpaceListModel(RoomListModel *sourceModel, QObject *parent)
    : QSortFilterProxyModel{parent}
{
    Q_ASSERT(sourceModel);
    setSourceModel(sourceModel);

    connect(this->sourceModel(), &QAbstractListModel::dataChanged, this, [this](const QModelIndex &, const QModelIndex &, QList<int> roles) {
        if (roles.contains(RoomListModel::IsChildSpaceRole)) {
            invalidate();
        }
        Q_EMIT countChanged();
    });

    setSortRole(RoomListModel::RoomIdRole);
    sort(0);
}

bool SortFilterSpaceListModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);
    return sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::IsSpaceRole).toBool()
        && sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::JoinStateRole).toString() != u"upgraded"_s
        && !sourceModel()->data(sourceModel()->index(source_row, 0), RoomListModel::IsChildSpaceRole).toBool();
}

bool SortFilterSpaceListModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const auto idLeft = sourceModel()->data(source_left, RoomListModel::RoomIdRole).toString();
    const auto idRight = sourceModel()->data(source_right, RoomListModel::RoomIdRole).toString();
    return idLeft < idRight;
}

#include "moc_sortfilterspacelistmodel.cpp"
