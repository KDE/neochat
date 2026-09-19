// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QConcatenateTablesProxyModel>
#include <qqmlintegration.h>

#include "completionlist.h"

/**
 * @class CompletionModel
 *
 * This class defines the available completions in the chat bar.
 *
 * The available completions are derived from one or more CompletionLists added
 * using the addList() function.
 */
class CompletionModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The UserListModel to adapt for completions.
     */
    Q_PROPERTY(QList<CompletionList *> completionLists READ completionLists WRITE setCompletionLists NOTIFY completionListsChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        TitleRole = Qt::DisplayRole, /**< The main text to show. */
        DescriptionRole = Qt::UserRole, /**< The subtitle text to show. */
        AvatarSourceRole, /**< The icon to show. */
        StartSequenceRole, /**< The text sequence that triggers this completion. */
        MatchSequencesRole, /**< The text sequence(s) to use for filter matching. */
        ReplaceStringRole, /**< The string to replace the completion with when completing. */
        HRefRole, /**< The link to add to the replaced string when completing. */
    };
    Q_ENUM(Roles)

    explicit CompletionModel(QObject *parent = nullptr);

    QList<CompletionList *> completionLists() const;
    void setCompletionLists(const QList<CompletionList *> &completionLists);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa EventRoles, QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void completionListsChanged();

private:
    QList<QPointer<CompletionList>> m_lists;
    std::optional<qsizetype> baseRowForList(CompletionList *list);
    void listCompletionsAdded(CompletionList *list, qsizetype first, qsizetype last);
    void listCompletionsRemoved(CompletionList *list, qsizetype first, qsizetype last);

    std::optional<Completion> completionAtRow(qsizetype row) const;
};
