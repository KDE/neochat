// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "userfiltermodel.h"

#include "userlistmodel.h"

bool UserFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    if (m_filterText.length() < 1) {
        return false;
    }
    return sourceModel()->data(sourceModel()->index(sourceRow, 0), UserListModel::DisplayNameRole).toString().contains(m_filterText, Qt::CaseInsensitive)
        || sourceModel()->data(sourceModel()->index(sourceRow, 0), UserListModel::UserIdRole).toString().contains(m_filterText, Qt::CaseInsensitive);
}

QString UserFilterModel::filterText() const
{
    return m_filterText;
}

void UserFilterModel::setFilterText(const QString &filterText)
{
    m_filterText = filterText;
    Q_EMIT filterTextChanged();
    invalidateFilter();
}
