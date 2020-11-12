/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QSortFilterProxyModel>

class SortFilterRoomListModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(RoomSortOrder roomSortOrder READ roomSortOrder WRITE setRoomSortOrder NOTIFY roomSortOrderChanged)

public:
    enum RoomSortOrder {
        Alphabetical,
        LastActivity,
        Categories,
    };
    Q_ENUM(RoomSortOrder)

    SortFilterRoomListModel(QObject *parent = nullptr);

    void setRoomSortOrder(RoomSortOrder sortOrder);
    RoomSortOrder roomSortOrder() const;

    Q_INVOKABLE void setFilterText(const QString &text);

    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

Q_SIGNALS:
    void roomSortOrderChanged();

private:
    RoomSortOrder m_sortOrder;
};
