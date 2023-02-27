// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
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
        StateEventsRole,
        AuthorListRole,
    };
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    /**
     * @brief QString aggregating the text of consecutive state events starting at row.
     *
     * If state events happen on different days they will be split into two aggregate
     * events.
     */
    [[nodiscard]] QString aggregateEventToString(int row) const;
    /**
     * @brief Return a list of consecutive state events starting at row.
     *
     * If state events happen on different days they will be split into two aggregate
     * events.
     */
    [[nodiscard]] QVariantList stateEventsList(int row) const;
    /**
     * @brief List of unique authors for the aggregate state events starting at row.
     */
    [[nodiscard]] QVariantList authorList(int row) const;
};
