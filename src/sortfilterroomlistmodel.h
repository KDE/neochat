/**
 * SPDX-FileCopyrightText: Tobias Fella <fella@posteo.de>
 *
 * SPDX-LicenseIdentifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QSortFilterProxyModel>

class SortFilterRoomListModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(RoomSortOrder roomSortOrder READ roomSortOrder WRITE setRoomSortOrder NOTIFY roomSortOrderChanged)
    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged)

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
    void sourceModelChanged();

private:
    RoomSortOrder m_sortOrder;
};
