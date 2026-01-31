// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "userfiltermodel.h"

#include "models/userlistmodel.h"

bool UserFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    if (!m_allowEmpty && m_filterText.isEmpty()) {
        return false;
    }
    if (m_membership != Quotient::Membership::Invalid) {
        if (sourceModel()->data(sourceModel()->index(sourceRow, 0), UserListModel::MembershipRole).value<Quotient::Membership>() != m_membership) {
            return false;
        }
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
    beginFilterChange();
    m_filterText = filterText;
    endFilterChange();
    Q_EMIT filterTextChanged();
}

bool UserFilterModel::allowEmpty() const
{
    return m_allowEmpty;
}

void UserFilterModel::setAllowEmpty(bool allowEmpty)
{
    beginFilterChange();
    m_allowEmpty = allowEmpty;
    endFilterChange();
    Q_EMIT allowEmptyChanged();
}

Quotient::Membership UserFilterModel::membership() const
{
    return m_membership;
}

void UserFilterModel::setMembership(const Quotient::Membership state)
{
    beginFilterChange();
    m_membership = state;
    endFilterChange();
    Q_EMIT membershipChanged();
}

#include "moc_userfiltermodel.cpp"
