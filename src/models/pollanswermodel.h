// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

class PollHandler;

/**
 * @class PollAnswerModel
 *
 * This class defines the model for visualising a list of answer to a poll.
 */
class PollAnswerModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        IdRole, /**< The ID of the answer. */
        TextRole, /**< The answer text. */
        CountRole, /**< The number of people who gave this answer. */
        LocalChoiceRole, /**< Whether this option was selected by the local user */
        IsWinnerRole, /**< Whether this option was selected by the local user */
    };
    Q_ENUM(Roles)

    explicit PollAnswerModel(PollHandler *parent);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent = {}) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;
};
