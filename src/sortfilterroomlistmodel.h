// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QSortFilterProxyModel>

class SortFilterRoomListModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(RoomSortOrder roomSortOrder READ roomSortOrder WRITE setRoomSortOrder NOTIFY roomSortOrderChanged)
    Q_PROPERTY(QString filterText READ filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(QVector<QString> activeSpaceRooms WRITE setActiveSpaceRooms)

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

    Q_INVOKABLE void setActiveSpaceRooms(QVector<QString> activeSpaceRooms);

protected:
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

Q_SIGNALS:
    void roomSortOrderChanged();
    void filterTextChanged();

private:
    RoomSortOrder m_sortOrder = Categories;
    QString m_filterText;
    QVector<QString> m_activeSpaceRooms;
};
