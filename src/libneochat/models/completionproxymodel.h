// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QSortFilterProxyModel>

/**
 * @class CompletionProxyModel
 *
 * A filter model to sort and filter completion results.
 *
 * This model is designed to work with multiple source models depending upon the
 * completion type.
 *
 * A model value will be shown if its primary or secondary role values start with
 * the filter text. The exception is if the full text perfectly matches
 * the primary filter role value in which case the completion ends (i.e. the filter
 * will return no results).
 *
 * @note The filter is primarily design to work with strings, therefore make sure
 *       that the source model roles that are to be filtered are strings.
 */
class CompletionProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    /**
     * @brief Wether a row should be shown or not.
     *
     * @sa QSortFilterProxyModel::filterAcceptsRow
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    /**
     * @brief Returns true if the value of source_left is less than source_right.
     *
     * @sa QSortFilterProxyModel::lessThan
     */
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    /**
     * @brief Get the current secondary filter role.
     */
    int secondaryFilterRole() const;

    /**
     * @brief Set the secondary filter role.
     *
     * Refer to the source model for what value corresponds to what role.
     */
    void setSecondaryFilterRole(int role);

    /**
     * @brief Get the current text being used to filter the source model.
     */
    QString filterText() const;

    /**
     * @brief Set the text to be used to filter the source model.
     */
    void setFilterText(const QString &filterText);

    /**
     * @brief Set the full text in the chatbar after the completion start.
     *
     * This is used to automatically end the completion if the user replicated the
     * primary filter role value perfectly.
     */
    void setFullText(const QString &fullText);

private:
    int m_secondaryFilterRole = -1;
    QString m_filterText;
    QString m_fullText;
};
