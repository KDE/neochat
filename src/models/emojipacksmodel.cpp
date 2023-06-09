// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "emojipacksmodel.h"

#include <QDebug>

#include <KLocalizedString>

#include "imagecontentmanager.h"

EmojiPacksModel::EmojiPacksModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant EmojiPacksModel::data(const QModelIndex &index, int role) const
{
    const auto row = index.row();

    const auto &category = ImageContentManager::instance().emojiPacks()[row];
    if (role == ImageContentPackRole::DisplayNameRole) {
        return category.description;
    }
    if (role == ImageContentPackRole::IconRole) {
        return category.icon;
    }
    if (role == ImageContentPackRole::IdentifierRole) {
        return category.stateKey;
    }
    if (role == ImageContentPackRole::IsStickerRole) {
        return false;
    }
    if (role == ImageContentPackRole::IsEmojiRole) {
        return true;
    }
    if (role == ImageContentPackRole::IsEmptyRole) {
        return false;
    }
    return {};
}

int EmojiPacksModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return ImageContentManager::instance().emojiPacks().count();
}

QHash<int, QByteArray> EmojiPacksModel::roleNames() const
{
    return {
        {ImageContentPackRole::DisplayNameRole, "displayName"},
        {ImageContentPackRole::IconRole, "icon"},
        {ImageContentPackRole::IdentifierRole, "identifier"},
    };
}
