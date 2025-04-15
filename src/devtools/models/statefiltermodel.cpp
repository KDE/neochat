// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "statefiltermodel.h"

#include "statemodel.h"

bool StateFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    // No need to run the check if there are no items in m_stateEventTypesFiltered.
    if (m_stateEventTypesFiltered.empty()) {
        return true;
    }
    return !m_stateEventTypesFiltered.contains(sourceModel()->data(sourceModel()->index(sourceRow, 0), StateModel::TypeRole).toString());
}

void StateFilterModel::addStateEventTypeFiltered(const QString &stateEventType)
{
    if (!m_stateEventTypesFiltered.contains(stateEventType)) {
        m_stateEventTypesFiltered.append(stateEventType);
        invalidateFilter();
    }
}

void StateFilterModel::removeStateEventTypeFiltered(const QString &stateEventType)
{
    if (m_stateEventTypesFiltered.contains(stateEventType)) {
        m_stateEventTypesFiltered.removeAll(stateEventType);
        invalidateFilter();
    }
}

#include "moc_statefiltermodel.cpp"
