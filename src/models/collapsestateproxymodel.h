// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "messageeventmodel.h"
#include <QSortFilterProxyModel>

class CollapseStateProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum Roles {
        AggregateDisplayRole = MessageEventModel::LastRole + 1,
    };
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    [[nodiscard]] QString aggregateEventToString(int row) const;
};
