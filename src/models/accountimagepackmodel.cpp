// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "accountimagepackmodel.h"

#include <KLocalizedString>

#include "imagecontentmanager.h"

QVariant AccountImagePackModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index);
    if (role == ImageContentPackRole::DisplayNameRole) {
        return i18n("Your Emojis");
    }
    if (role == ImageContentPackRole::IconRole) {
        return imageContentManager.accountImagesAvatar();
    }
    if (role == ImageContentPackRole::IdentifierRole) {
        return QStringLiteral("account");
    }
    if (role == ImageContentPackRole::IsEmojiRole) {
        for (const auto &image : imageContentManager.accountImages()) {
            if (!image.usage || image.usage->isEmpty() || image.usage->contains(QStringLiteral("emoticon"))) {
                return true;
            }
        }
        return false;
    }
    if (role == ImageContentPackRole::IsStickerRole) {
        for (const auto &image : imageContentManager.accountImages()) {
            if (!image.usage || image.usage->isEmpty() || image.usage->contains(QStringLiteral("sticker"))) {
                return true;
            }
        }
        return false;
    }
    if (role == ImageContentPackRole::IsEmptyRole) {
        return imageContentManager.accountImages().size() == 0;
    }
    return {};
}

int AccountImagePackModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return ImageContentManager::instance().accountImages().size() > 0 ? 1 : 0;
}

QHash<int, QByteArray> AccountImagePackModel::roleNames() const
{
    return {
        {ImageContentPackRole::DisplayNameRole, "displayName"},
        {ImageContentPackRole::IconRole, "emoji"},
        {ImageContentPackRole::IdentifierRole, "identifier"},
    };
}

AccountImagePackModel::AccountImagePackModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&ImageContentManager::instance(), &ImageContentManager::accountImagesChanged, this, [this]() {
        beginResetModel();
        endResetModel();
    });
}
