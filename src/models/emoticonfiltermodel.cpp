// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "emoticonfiltermodel.h"

#include "accountemoticonmodel.h"

EmoticonFilterModel::EmoticonFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool EmoticonFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto stickerUsage = sourceModel()->data(sourceModel()->index(sourceRow, 0), AccountEmoticonModel::IsStickerRole).toBool();
    auto emojiUsage = sourceModel()->data(sourceModel()->index(sourceRow, 0), AccountEmoticonModel::IsEmojiRole).toBool();
    return (stickerUsage && m_showStickers) || (emojiUsage && m_showEmojis);
}

bool EmoticonFilterModel::showStickers() const
{
    return m_showStickers;
}

void EmoticonFilterModel::setShowStickers(bool showStickers)
{
    beginResetModel();
    m_showStickers = showStickers;
    endResetModel();
    Q_EMIT showStickersChanged();
}

bool EmoticonFilterModel::showEmojis() const
{
    return m_showEmojis;
}

void EmoticonFilterModel::setShowEmojis(bool showEmojis)
{
    beginResetModel();
    m_showEmojis = showEmojis;
    endResetModel();
    Q_EMIT showEmojisChanged();
}
