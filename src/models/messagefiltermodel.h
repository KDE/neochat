// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QQmlEngine>
#include <QSortFilterProxyModel>

#include "messageeventmodel.h"
#include "timelinemodel.h"

/**
 * @class MessageFilterModel
 *
 * This model filters out any messages that should be hidden.
 *
 * Deleted messages are only hidden if the user hasn't set them to be shown.
 *
 * The model also contains the roles and functions to support aggregating multiple
 * consecutive state events into a single delegate. The state events must all happen
 * on the same day to be aggregated.
 */
class MessageFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        AggregateDisplayRole = MessageEventModel::LastRole + 1, /**< Single line aggregation of all the state events. */
        StateEventsRole, /**< List of state events in the aggregated state. */
        AuthorListRole, /**< List of the first 5 unique authors of the aggregated state event. */
        ExcessAuthorsRole, /**< The number of unique authors beyond the first 5. */
        LastRole, // Keep this last
    };

    explicit MessageFilterModel(QObject *parent = nullptr, TimelineModel *sourceModel = nullptr);

    /**
     * @brief Custom filter function to remove hidden messages.
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

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
     * @brief List of the first 5 unique authors for the aggregate state events starting at row.
     */
    [[nodiscard]] QVariantList authorList(int row) const;

    /**
     * @brief The number of unique authors beyond the first 5 for the aggregate state events starting at row.
     */
    [[nodiscard]] QString excessAuthors(int row) const;
};
