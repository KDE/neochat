// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QQmlEngine>
#include <QSortFilterProxyModel>

#include "models/timelinemessagemodel.h"

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

    /**
     * @brief The model index of the read marker.
     */
    Q_PROPERTY(QPersistentModelIndex readMarkerIndex READ readMarkerIndex NOTIFY readMarkerIndexChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        AggregateDisplayRole = TimelineMessageModel::LastRole + 1, /**< Single line aggregation of all the state events. */
        StateEventsRole, /**< List of state events in the aggregated state. */
        AuthorListRole, /**< List of the first 5 unique authors of the aggregated state event. */
        ExcessAuthorsRole, /**< The number of unique authors beyond the first 5. */
        LastRole, // Keep this last
    };

    QPersistentModelIndex readMarkerIndex() const;

    explicit MessageFilterModel(QObject *parent = nullptr, QAbstractItemModel *sourceModel = nullptr);

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

    /**
     * @brief Get the QModelIndex the given event ID in the model.
     */
    Q_INVOKABLE QModelIndex indexforEventId(const QString &eventId) const;

    static void setShowAllEvents(bool enabled);
    static void setShowDeletedMessages(bool enabled);

Q_SIGNALS:
    /**
     * @brief Emitted when the reader marker index is changed.
     */
    void readMarkerIndexChanged();

private:
    static bool m_showAllEvents;
    static bool m_showDeletedMessages;

    bool eventIsVisible(int sourceRow, const QModelIndex &sourceParent) const;

    bool showAuthor(QModelIndex index) const;

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
