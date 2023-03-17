// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "devicesproxymodel.h"
#include "devicesmodel.h"

int DevicesProxyModel::type() const
{
    return m_type;
}
void DevicesProxyModel::setType(int type)
{
    m_type = type;
    Q_EMIT typeChanged();
}

bool DevicesProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)
    return sourceModel()->data(sourceModel()->index(source_row, 0), DevicesModel::Type).toInt() == m_type;
}
DevicesProxyModel::DevicesProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_type(0)
{
    setSortRole(DevicesModel::LastTimestamp);
    sort(0, Qt::DescendingOrder);
}
