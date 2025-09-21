// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roomsortparametermodel.h"

using namespace Qt::StringLiterals;

RoomSortParameterModel::RoomSortParameterModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_currentParameters = RoomSortParameter::currentParameterList();
}

RoomSortParameterModel::RoomSortParameterModel(QList<RoomSortParameter::Parameter> parameters, QObject *parent)
    : QAbstractListModel(parent)
{
    m_currentParameters = parameters;
}

QList<int> RoomSortParameterModel::currentParameterList() const
{
    QList<int> intList;
    std::transform(m_currentParameters.constBegin(), m_currentParameters.constEnd(), std::back_inserter(intList), [](RoomSortParameter::Parameter param) {
        return static_cast<int>(param);
    });
    return intList;
}

QVariant RoomSortParameterModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount()) {
        return {};
    }

    const auto parameter = m_currentParameters.at(index.row());
    if (role == Name) {
        return RoomSortParameter::parameterName(parameter);
    }
    if (role == Description) {
        return RoomSortParameter::parameterDescription(parameter);
    }
    return {};
}

int RoomSortParameterModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_currentParameters.size();
}

QHash<int, QByteArray> RoomSortParameterModel::roleNames() const
{
    return {
        {Name, "name"},
        {Description, "description"},
    };
}

void RoomSortParameterModel::addParameter(RoomSortParameter::Parameter parameter)
{
    if (m_currentParameters.contains(parameter)) {
        return;
    }

    beginInsertRows({}, rowCount(), rowCount());
    m_currentParameters.append(parameter);
    endInsertRows();
}

void RoomSortParameterModel::removeRow(int row)
{
    if (rowCount() <= 1 || row < 0 || row >= rowCount()) {
        return;
    }

    beginRemoveRows({}, row, row);
    m_currentParameters.remove(row);
    endRemoveRows();
}

void RoomSortParameterModel::moveRowUp(int row)
{
    if (row < 1 || row >= rowCount()) {
        return;
    }

    beginMoveRows({}, row, row, {}, row - 1);
    m_currentParameters.move(row, row - 1);
    endMoveRows();
}

void RoomSortParameterModel::moveRowDown(int row)
{
    if (row < 0 || row >= rowCount() - 1) {
        return;
    }

    beginMoveRows({}, row, row, {}, row + 2);
    m_currentParameters.move(row, row + 1);
    endMoveRows();
}

RoomSortParameterModel *RoomSortParameterModel::allParameterModel() const
{
    return new RoomSortParameterModel(RoomSortParameter::allParameterList());
}
