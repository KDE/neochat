// SPDX-FileCopyrightText: 2022 Snehit Sah <hi@snehit.dev>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QQmlEngine>
#include <QSortFilterProxyModel>

/**
 * @class SortFilterSpaceListModel
 *
 * This model sorts and filters the space list.
 *
 * The spaces are sorted by their matrix ID. The filter only shows space rooms,
 * but filters out upgraded spaces.
 */
class SortFilterSpaceListModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The number of spaces in the model.
     */
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    explicit SortFilterSpaceListModel(QObject *parent = nullptr);

Q_SIGNALS:
    void countChanged();

protected:
    /**
     * @brief Returns true if the value of source_left is less than source_right.
     *
     * @sa QSortFilterProxyModel::lessThan
     */
    [[nodiscard]] bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    /**
     * @brief Whether a row should be shown out or not.
     *
     * @sa QSortFilterProxyModel::filterAcceptsRow
     */
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};
