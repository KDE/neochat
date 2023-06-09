// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imagecontentfiltermodel.h"

#include "imagecontentmanager.h"

ImageContentFilterModel::ImageContentFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool ImageContentFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto index = sourceModel()->index(sourceRow, 0);
    return index.data(ImageContentRole::IsEmojiRole).toBool() && emojis() || index.data(ImageContentRole::IsStickerRole).toBool() && stickers();
}

bool ImageContentFilterModel::stickers() const
{
    return m_stickers;
}

void ImageContentFilterModel::setStickers(bool stickers)
{
    m_stickers = stickers;
    Q_EMIT stickersChanged();
    invalidateFilter();
}

bool ImageContentFilterModel::emojis() const
{
    return m_emojis;
}

void ImageContentFilterModel::setEmojis(bool emojis)
{
    m_emojis = emojis;
    Q_EMIT emojisChanged();
    invalidateFilter();
}
