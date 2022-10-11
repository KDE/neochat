// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QSortFilterProxyModel>
#include "roomlistmodel.h"

class SortFilterRoomListModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(RoomSortOrder roomSortOrder READ roomSortOrder WRITE setRoomSortOrder NOTIFY roomSortOrderChanged)
    Q_PROPERTY(QString filterText READ filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(QString activeSpaceId READ activeSpaceId WRITE setActiveSpaceId NOTIFY activeSpaceIdChanged)

public:
    enum RoomSortOrder {
        Alphabetical,
        LastActivity,
        Categories,
    };
    Q_ENUM(RoomSortOrder)

    SortFilterRoomListModel(QObject *parent = nullptr);

    void setRoomSortOrder(RoomSortOrder sortOrder);
    [[nodiscard]] RoomSortOrder roomSortOrder() const;

    void setFilterText(const QString &text);
    [[nodiscard]] QString filterText() const;

    [[nodiscard]] bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    QString activeSpaceId() const;
    void setActiveSpaceId(const QString &spaceId);

protected:
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

Q_SIGNALS:
    void roomSortOrderChanged();
    void filterTextChanged();
    void activeSpaceIdChanged();

private:
    RoomSortOrder m_sortOrder = Categories;
    QString m_filterText;
    QString m_activeSpaceId;

    bool prioritiesCmp(const QVector<RoomListModel::EventRoles>& priorities, const QModelIndex &left, const QModelIndex &right) const;
    bool roleCmp(RoomListModel::EventRoles role, const QVariant& left, const QVariant& right) const;
};
