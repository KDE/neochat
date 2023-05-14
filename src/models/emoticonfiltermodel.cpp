// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "emoticonfiltermodel.h"

#include "accountstickermodel.h"

EmoticonFilterModel::EmoticonFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool EmoticonFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto stickerUsage = sourceModel()->data(sourceModel()->index(sourceRow, 0), AccountStickerModel::IsStickerRole).toBool();
    auto emojiUsage = sourceModel()->data(sourceModel()->index(sourceRow, 0), AccountStickerModel::IsEmojiRole).toBool();
    return (stickerUsage && m_showStickers) || (emojiUsage && m_showEmojis);
}

bool EmoticonFilterModel::showStickers() const
{
    return m_showStickers;
}

void EmoticonFilterModel::setShowStickers(bool showStickers)
{
    m_showStickers = showStickers;
    Q_EMIT showStickersChanged();
}

bool EmoticonFilterModel::showEmojis() const
{
    return m_showEmojis;
}

void EmoticonFilterModel::setShowEmojis(bool showEmojis)
{
    m_showEmojis = showEmojis;
    Q_EMIT showEmojisChanged();
}
