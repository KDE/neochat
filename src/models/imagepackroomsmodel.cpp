// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imagepackroomsmodel.h"

#include <Quotient/connection.h>
#include <Quotient/room.h>

#include "controller.h"
#include "imagecontentmanager.h"

using namespace Quotient;

ImagePackRoomsModel::ImagePackRoomsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&imageContentManager, &ImageContentManager::globalPacksChanged, this, [this]() {
        beginResetModel();
        endResetModel();
    });
}

QVariant ImagePackRoomsModel::data(const QModelIndex &index, int role) const
{
    const auto &row = index.row();
    const auto &packKey = imageContentManager.globalPacks()[row];
    if (!imageContentManager.roomImagePacks().contains(packKey.first) || !imageContentManager.roomImagePacks()[packKey.first].contains(packKey.second)) {
        return false;
    }
    const auto &pack = imageContentManager.roomImagePacks()[packKey.first][packKey.second];
    if (role == ImageContentPackRole::DisplayNameRole) {
        return pack.description;
    }
    if (role == ImageContentPackRole::IconRole) {
        return pack.icon;
    }
    if (role == ImageContentPackRole::IdentifierRole) {
        return QStringLiteral("%1@%2").arg(pack.roomId, pack.stateKey);
    }
    if (role == ImageContentPackRole::IsStickerRole) {
        return pack.type == ImagePackDescription::Sticker;
    }
    if (role == ImageContentPackRole::IsEmojiRole) {
        return pack.type == ImagePackDescription::Emoji || pack.type == ImagePackDescription::CustomEmoji;
    }
    if (role == ImageContentPackRole::IsEmptyRole) {
        return imageContentManager.roomImages()[packKey].size() == 0;
    }
    if (role == ImageContentPackRole::IsGlobalPackRole) {
        return true;
    }
    return {};
}

int ImagePackRoomsModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return imageContentManager.globalPacks().size();
}

QHash<int, QByteArray> ImagePackRoomsModel::roleNames() const
{
    return {
        {ImageContentPackRole::DisplayNameRole, "displayName"},
        {ImageContentPackRole::IconRole, "emoji"}, // TODO rename
        {ImageContentPackRole::IdentifierRole, "identifier"},
    };
}
