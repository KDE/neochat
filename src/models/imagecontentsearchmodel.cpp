// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imagecontentsearchmodel.h"
#include "imagecontentmanager.h"
#include "models/allimagecontentmodel.h"

#include <QDebug>

ImageContentSearchModel::ImageContentSearchModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(new AllImageContentModel(this));
}

bool ImageContentSearchModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    return sourceModel()->index(sourceRow, 0).data(ImageContentRole::DisplayNameRole).toString().contains(m_searchText);
}

QString ImageContentSearchModel::searchText() const
{
    return m_searchText;
}

void ImageContentSearchModel::setSearchText(QString searchText)
{
    m_searchText = searchText;
    Q_EMIT searchTextChanged();
    invalidateFilter();
}
