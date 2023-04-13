// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "messageeventmodel.h"
#include <QSortFilterProxyModel>

/**
 * @class CollapseStateProxyModel
 *
 * This model aggregates multiple sequential state events into a single entry.
 *
 * Events are only aggregated if they happened on the same day.
 *
 * @sa MessageEventModel
 */
class CollapseStateProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        AggregateDisplayRole = MessageEventModel::LastRole + 1, /**< Single line aggregation of all the state events. */
        StateEventsRole, /**< List of state events in the aggregated state. */
        AuthorListRole, /**< List of unique authors of the aggregated state event. */
    };

    /**
     * @brief Whether a row should be shown out or not.
     *
     * @sa QSortFilterProxyModel::filterAcceptsRow
     */
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QSortFilterProxyModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractProxyModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    /**
     * @brief Aggregation of the text of consecutive state events starting at row.
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
