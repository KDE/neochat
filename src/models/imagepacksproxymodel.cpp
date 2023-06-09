// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imagepacksproxymodel.h"

#include "imagecontentmanager.h"
#include "imagepacksmodel.h"
#include "neochatroom.h"

ImagePacksProxyModel::ImagePacksProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(new ImagePacksModel(this));
}

bool ImagePacksProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    const auto &identifier = sourceModel()->data(sourceModel()->index(sourceRow, 0), ImageContentPackRole::IdentifierRole).toString();
    if (identifier.contains(u'@')) {
        const auto roomId = identifier.split(u'@')[0];
        if (static_cast<ImagePacksModel *>(sourceModel())->currentRoom() && roomId == static_cast<ImagePacksModel *>(sourceModel())->currentRoom()->id()
            && sourceModel()->data(sourceModel()->index(sourceRow, 0), ImageContentPackRole::IsGlobalPackRole).toBool()) {
            // Hide this pack, as it's already exposed as a global pack
            return false;
        }
    }
    return ((sourceModel()->data(sourceModel()->index(sourceRow, 0), ImageContentPackRole::IsEmojiRole).toBool() && emojis())
            || (sourceModel()->data(sourceModel()->index(sourceRow, 0), ImageContentPackRole::IsStickerRole).toBool() && stickers()));
}

bool ImagePacksProxyModel::stickers() const
{
    return m_stickers;
}

void ImagePacksProxyModel::setStickers(bool stickers)
{
    m_stickers = stickers;
    Q_EMIT stickersChanged();
    invalidateFilter();
}

bool ImagePacksProxyModel::emojis() const
{
    return m_emojis;
}

void ImagePacksProxyModel::setEmojis(bool emojis)
{
    m_emojis = emojis;
    Q_EMIT emojisChanged();
    invalidateFilter();
}

NeoChatRoom *ImagePacksProxyModel::currentRoom() const
{
    return static_cast<ImagePacksModel *>(sourceModel())->currentRoom();
}

void ImagePacksProxyModel::setCurrentRoom(NeoChatRoom *currentRoom)
{
    beginResetModel();
    static_cast<ImagePacksModel *>(sourceModel())->setCurrentRoom(currentRoom);
    endResetModel();
    Q_EMIT currentRoomChanged();
}
