// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <KLazyLocalizedString>
#include <qtmetamacros.h>

#include "enums/roomsortparameter.h"

/**
 * @class RoomSortParameterModel
 *
 * This model is used to visualize and modify the current sorting priorities.
 */
class RoomSortParameterModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        Name = Qt::DisplayRole, /**< The name of the sort parameter. */
        Description, /**< The description of the sort parameter. */
    };
    Q_ENUM(Roles)

    explicit RoomSortParameterModel(QObject *parent = nullptr);
    explicit RoomSortParameterModel(QList<RoomSortParameter::Parameter> parameters, QObject *parent = nullptr);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa EventRoles, QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Add the given parameter to the model.
     *
     * If the Parameter is already in the model nothing will happen.
     */
    Q_INVOKABLE void addParameter(RoomSortParameter::Parameter parameter);

    /**
     * @brief Remove the given row from the model.
     */
    Q_INVOKABLE void removeRow(int row);

    /**
     * @brief Move the given row up one.
     */
    Q_INVOKABLE void moveRowUp(int row);

    /**
     * @brief Move the given row down one.
     */
    Q_INVOKABLE void moveRowDown(int row);

    /**
     * @brief Save the current model parameters as a custom sort order.
     */
    Q_INVOKABLE void saveParameterList();

    /**
     * @brief Return a RoomSortParameterModel with all available parameters.
     */
    Q_INVOKABLE RoomSortParameterModel *allParameterModel() const;

private:
    QList<RoomSortParameter::Parameter> m_currentParameters;
};
