// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "roomimagepacksmodel.h"
#include "imagecontentmanager.h"

#include "neochatroom.h"

NeoChatRoom *RoomImagePacksModel::currentRoom() const
{
    return m_currentRoom;
}

void RoomImagePacksModel::setCurrentRoom(NeoChatRoom *currentRoom)
{
    if (m_currentRoom == currentRoom) {
        return;
    }
    if (m_currentRoom) {
        disconnect(m_currentRoom, nullptr, this, nullptr);
    }
    beginResetModel();
    m_currentRoom = currentRoom;
    endResetModel();
    Q_EMIT currentRoomChanged();
}

QVariant RoomImagePacksModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index);
    if (!m_currentRoom) {
        return {};
    }
    const auto row = index.row();
    const auto &packs = ImageContentManager::instance().roomImagePacks()[m_currentRoom->id()].values();
    const auto &pack = packs[row];
    if (role == ImageContentPackRole::DisplayNameRole) {
        return pack.description;
    }
    if (role == ImageContentPackRole::IconRole) {
        return pack.icon;
    }
    if (role == ImageContentPackRole::IdentifierRole) {
        return QStringLiteral("%1@%2").arg(m_currentRoom->id(), pack.stateKey);
    }
    if (role == ImageContentPackRole::IsStickerRole) {
        return pack.type == ImagePackDescription::Sticker || pack.type == ImagePackDescription::Both;
    }
    if (role == ImageContentPackRole::IsEmojiRole) {
        return pack.type == ImagePackDescription::Emoji || pack.type == ImagePackDescription::CustomEmoji || pack.type == ImagePackDescription::Both;
    }
    if (role == ImageContentPackRole::IsEmptyRole) {
        return imageContentManager.roomImages()[{m_currentRoom->id(), pack.stateKey}].isEmpty();
    }
    return {};
}

int RoomImagePacksModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    if (!m_currentRoom) {
        return {};
    }
    return ImageContentManager::instance().roomImagePacks()[m_currentRoom->id()].size();
}

QHash<int, QByteArray> RoomImagePacksModel::roleNames() const
{
    return {
        {ImageContentPackRole::DisplayNameRole, "displayName"},
        {ImageContentPackRole::IconRole, "emoji"},
        {ImageContentPackRole::IdentifierRole, "identifier"},
    };
}

RoomImagePacksModel::RoomImagePacksModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&ImageContentManager::instance(), &ImageContentManager::roomImagePacksChanged, this, [this](NeoChatRoom *room) {
        if (room != m_currentRoom) {
            return;
        }
        beginResetModel();
        endResetModel();
    });
}
