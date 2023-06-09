// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "recentimagecontentproxymodel.h"

#include "imagecontentmanager.h"
#include "recentimagecontentmodel.h"

RecentImageContentProxyModel::RecentImageContentProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(new RecentImageContentModel(this));
    sort(0);
}

bool RecentImageContentProxyModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    return sourceLeft.data(ImageContentRole::UsageCountRole).toInt() > sourceRight.data(ImageContentRole::UsageCountRole).toInt();
}
