// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "completionproxymodel.h"
#include <QDebug>

#include "neochatroom.h"

bool CompletionProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    if (m_filterText.isEmpty()) {
        return false;
    }
    return (sourceModel()->data(sourceModel()->index(sourceRow, 0), filterRole()).toString().startsWith(m_filterText, Qt::CaseInsensitive)
            && !m_fullText.startsWith(sourceModel()->data(sourceModel()->index(sourceRow, 0), filterRole()).toString()))
        || (m_secondaryFilterRole != -1
            && sourceModel()
                   ->data(sourceModel()->index(sourceRow, 0), secondaryFilterRole())
                   .toString()
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
                   .startsWith(QStringView(m_filterText).sliced(1), Qt::CaseInsensitive));
#else
                   .startsWith(m_filterText.midRef(1), Qt::CaseInsensitive));
#endif
}

bool CompletionProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (m_secondaryFilterRole == -1)
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    bool left_primary = sourceModel()->data(source_left, filterRole()).toString().startsWith(m_filterText, Qt::CaseInsensitive);
    bool right_primary = sourceModel()->data(source_right, filterRole()).toString().startsWith(m_filterText, Qt::CaseInsensitive);
    if (left_primary != right_primary)
        return left_primary;
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

int CompletionProxyModel::secondaryFilterRole() const
{
    return m_secondaryFilterRole;
}

void CompletionProxyModel::setSecondaryFilterRole(int role)
{
    m_secondaryFilterRole = role;
    Q_EMIT secondaryFilterRoleChanged();
}

QString CompletionProxyModel::filterText() const
{
    return m_filterText;
}

void CompletionProxyModel::setFilterText(const QString &filterText)
{
    m_filterText = filterText;
    Q_EMIT filterTextChanged();
}

void CompletionProxyModel::setFullText(const QString &fullText)
{
    m_fullText = fullText;
}
