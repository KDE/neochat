// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imagepacksmodel.h"

#include "models/accountimagepackmodel.h"
#include "models/emojipacksmodel.h"
#include "models/historyimagepackmodel.h"
#include "models/imagepackroomsmodel.h"
#include "models/roomimagepacksmodel.h"

ImagePacksModel::ImagePacksModel(QObject *parent)
    : QConcatenateTablesProxyModel(parent)
{
    addSourceModel(new HistoryImagePackModel(parent));
    addSourceModel(new AccountImagePackModel(parent));
    m_roomImagePacksModel = new RoomImagePacksModel(parent);
    addSourceModel(m_roomImagePacksModel);
    addSourceModel(new ImagePackRoomsModel(parent));
    addSourceModel(new EmojiPacksModel(parent));
}

// TODO required?
QHash<int, QByteArray> ImagePacksModel::roleNames() const
{
    return {
        {ImageContentPackRole::DisplayNameRole, "displayName"},
        {ImageContentPackRole::IconRole, "icon"},
        {ImageContentPackRole::IdentifierRole, "identifier"},
    };
}

NeoChatRoom *ImagePacksModel::currentRoom() const
{
    return m_roomImagePacksModel->currentRoom();
}

void ImagePacksModel::setCurrentRoom(NeoChatRoom *currentRoom)
{
    m_roomImagePacksModel->setCurrentRoom(currentRoom);
    Q_EMIT currentRoomChanged();
}

