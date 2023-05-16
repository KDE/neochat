// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>

class NeoChatUser;

/**
 * @class ReactionModel
 *
 * This class defines the model for visualising a list of reactions to an event.
 */
class ReactionModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /**
     * @brief Definition of an reaction.
     */
    struct Reaction {
        QString reaction; /**< The reaction emoji. */
        QVariantList authors; /**< The list of authors who sent the given reaction. */
    };

    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        TextRole = Qt::DisplayRole, /**< The text to show in the reaction. */
        ReactionRole, /**< The reaction emoji. */
        ToolTipRole, /**< The tool tip to show for the reaction. */
        AuthorsRole, /**< The list of authors who sent the given reaction. */
        HasLocalUser, /**< Whether the local user is in the list of authors. */
    };

    ReactionModel(QObject *parent = nullptr, QList<Reaction> reactions = {}, NeoChatUser *localUser = nullptr);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Set the reactions data in the model.
     */
    void setReactions(QList<Reaction> reactions);

private:
    QList<Reaction> m_reactions;

    NeoChatUser *m_localUser;
};
Q_DECLARE_METATYPE(ReactionModel *)
