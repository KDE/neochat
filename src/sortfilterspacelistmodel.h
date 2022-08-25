// SPDX-FileCopyrightText: 2022 Snehit Sah <hi@snehit.dev>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QSortFilterProxyModel>

class SortFilterSpaceListModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit SortFilterSpaceListModel(QObject *parent = nullptr);

    [[nodiscard]] QString activeSpaceId() const;

    [[nodiscard]] bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

Q_SIGNALS:
    void activeSpaceIdChanged(QString &activeSpaceId);

protected:
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};
