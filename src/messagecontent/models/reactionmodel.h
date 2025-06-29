// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <Quotient/events/reactionevent.h>
#include <Quotient/roommember.h>

namespace Quotient
{
class RoomMessageEvent;
}

class MessageContentModel;
class NeoChatRoom;

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
        QStringList authors; /**< The list of authors who sent the given reaction. */
    };

    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        TextContentRole = Qt::DisplayRole, /**< The text to show in the reaction. */
        ReactionRole, /**< The reaction emoji. */
        ToolTipRole, /**< The tool tip to show for the reaction. */
        HasLocalMember, /**< Whether the local member is in the list of authors. */
    };

    explicit ReactionModel(MessageContentModel *parent, const QString &eventId, NeoChatRoom *room);

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

Q_SIGNALS:
    /**
     * @brief The reactions in the model have been updated.
     */
    void reactionsUpdated();

private:
    QPointer<NeoChatRoom> m_room;
    QString m_eventId;
    QList<Reaction> m_reactions;
    QMap<QString, QString> m_shortcodes;

    void updateReactions();
    QString reactionText(QString text) const;
};
Q_DECLARE_METATYPE(ReactionModel *)
