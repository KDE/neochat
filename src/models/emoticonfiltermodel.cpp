// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "emoticonfiltermodel.h"

#include "accountemoticonmodel.h"
#include "stickermodel.h"

EmoticonFilterModel::EmoticonFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &EmoticonFilterModel::sourceModelChanged, this, [this]() {
        if (dynamic_cast<StickerModel *>(sourceModel())) {
            m_stickerRole = StickerModel::IsStickerRole;
            m_emojiRole = StickerModel::IsEmojiRole;
        } else {
            m_stickerRole = AccountEmoticonModel::IsStickerRole;
            m_emojiRole = AccountEmoticonModel::IsEmojiRole;
        }
    });
}

bool EmoticonFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    auto stickerUsage = sourceModel()->data(sourceModel()->index(sourceRow, 0), m_stickerRole).toBool();
    auto emojiUsage = sourceModel()->data(sourceModel()->index(sourceRow, 0), m_emojiRole).toBool();
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

#include "moc_emoticonfiltermodel.cpp"
