// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QSortFilterProxyModel>

#include "chattextitemhelper.h"

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
    QML_ELEMENT

    /**
     * @brief The QML text Item that completions are being provided for.
     */
    Q_PROPERTY(ChatTextItemHelper *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

    /**
     * @brief Whether a completion is currently being typed.
     */
    Q_PROPERTY(bool isCompleting READ isCompleting NOTIFY isCompletingChanged)

public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    ChatTextItemHelper *textItem() const;
    void setTextItem(ChatTextItemHelper *textItem);

    bool isCompleting() const;

    Q_INVOKABLE void ignoreCurrentCompletion();

    Q_INVOKABLE void insertCompletion(const QString &text, const QUrl &link);

    /**
     * @brief Wether a row should be shown or not.
     *
     * @sa QSortFilterProxyModel::filterAcceptsRow
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

Q_SIGNALS:
    void textItemChanged();
    void isCompletingChanged();

private:
    QPointer<ChatTextItemHelper> m_textItem;

    bool m_ignoreCurrentCompletion = false;
    void updateFilterText();
    QString m_filterText;
};
