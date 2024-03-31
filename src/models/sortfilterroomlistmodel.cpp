// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sortfilterroomlistmodel.h"

#include "roomlistmodel.h"

SortFilterRoomListModel::SortFilterRoomListModel(RoomListModel *sourceModel, QObject *parent)
    : QSortFilterProxyModel(parent)
{
    Q_ASSERT(sourceModel);
    setSourceModel(sourceModel);

    sort(0);
    invalidateFilter();
    connect(this, &SortFilterRoomListModel::filterTextChanged, this, [this]() {
        invalidateFilter();
    });
    connect(this, &SortFilterRoomListModel::sourceModelChanged, this, [this]() {
        connect(this->sourceModel(), &QAbstractListModel::rowsInserted, this, &SortFilterRoomListModel::invalidateRowsFilter);
        connect(this->sourceModel(), &QAbstractListModel::rowsRemoved, this, &SortFilterRoomListModel::invalidateRowsFilter);
    });
}

void SortFilterRoomListModel::setFilterText(const QString &text)
{
    m_filterText = text;
    Q_EMIT filterTextChanged();
}

QString SortFilterRoomListModel::filterText() const
{
    return m_filterText;
}

bool SortFilterRoomListModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    if (sourceModel()->data(index, RoomListModel::JoinStateRole).toString() == QStringLiteral("upgraded")
        && dynamic_cast<RoomListModel *>(sourceModel())->connection()->room(sourceModel()->data(index, RoomListModel::ReplacementIdRole).toString())) {
        return false;
    }

    return sourceModel()->data(index, RoomListModel::DisplayNameRole).toString().contains(m_filterText, Qt::CaseInsensitive)
        && sourceModel()->data(index, RoomListModel::IsSpaceRole).toBool() == false;
}

#include "moc_sortfilterroomlistmodel.cpp"
