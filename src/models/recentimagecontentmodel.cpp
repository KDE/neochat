// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "recentimagecontentmodel.h"

#include "imagecontentmanager.h"

RecentImageContentModel::RecentImageContentModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&ImageContentManager::instance(), &ImageContentManager::recentEmojisChanged, this, [this]() {
        beginResetModel();
        endResetModel();
    });
}

QVariant RecentImageContentModel::data(const QModelIndex &index, int role) const
{
    const auto &recent = ImageContentManager::instance().recentEmojis();
    const auto row = index.row();
    const bool isCustom = recent.keys()[row].startsWith(QLatin1Char(':'));

    if (role == ImageContentRole::DisplayNameRole) {
        if (isCustom) {
            return imageContentManager.bodyForShortCode(recent.keys()[row].mid(1).chopped(1));
        }
        return ImageContentManager::instance().emojiForText(recent.keys()[row]).displayName;
    }
    if (role == ImageContentRole::EmojiRole) {
        if (isCustom) {
            return QStringLiteral("<img src=\"%1\" width=\"32\" height=\"32\"/>")
                .arg(imageContentManager.mxcForShortCode(recent.keys()[row].mid(1).chopped(1)));
        }
        return recent.keys()[row];
    }
    if (role == ImageContentRole::UsageCountRole) {
        return recent[recent.keys()[row]];
    }
    if (role == ImageContentRole::ShortCodeRole) {
        return recent.keys()[row].mid(1).chopped(1);
    }
    if (role == ImageContentRole::IsCustomRole) {
        return isCustom;
    }
    if (role == ImageContentRole::IsEmojiRole) {
        if (!isCustom)
            return true;
        return imageContentManager.isEmojiShortCode(recent.keys()[row].mid(1).chopped(1));
    }
    if (role == ImageContentRole::IsStickerRole) {
        if (!isCustom)
            return false;
        return imageContentManager.isStickerShortCode(recent.keys()[row].mid(1).chopped(1));
    }
    return {};
}

int RecentImageContentModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return ImageContentManager::instance().recentEmojis().size();
}

QHash<int, QByteArray> RecentImageContentModel::roleNames() const
{
    return {
        {ImageContentRole::DisplayNameRole, "displayName"},
        {ImageContentRole::EmojiRole, "text"},
        {ImageContentRole::UsageCountRole, "usageCount"},
        {ImageContentRole::ShortCodeRole, "shortCode"},
        {ImageContentRole::IsCustomRole, "isCustom"},
    };
}
