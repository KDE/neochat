// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "neochatroom.h"
#include <QAbstractListModel>
#include <QQmlEngine>
#include <Quotient/events/reactionevent.h>

namespace Quotient
{
class User;
}

/**
 * @class ReactionModel
 *
 * This class defines the model for visualising a list of reactions to an event.
 */
class ReactionModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

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
        TextContentRole = Qt::DisplayRole, /**< The text to show in the reaction. */
        ReactionRole, /**< The reaction emoji. */
        ToolTipRole, /**< The tool tip to show for the reaction. */
        AuthorsRole, /**< The list of authors who sent the given reaction. */
        HasLocalUser, /**< Whether the local user is in the list of authors. */
    };

    explicit ReactionModel(const Quotient::RoomMessageEvent *event, NeoChatRoom *room);

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

private:
    QPointer<NeoChatRoom> m_room;
    const Quotient::RoomMessageEvent *m_event;
    QList<Reaction> m_reactions;
    QMap<QString, QString> m_shortcodes;

    void updateReactions();
    QString reactionText(QString text) const;
};
Q_DECLARE_METATYPE(ReactionModel *)
