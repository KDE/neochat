// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "allimagecontentmodel.h"

#include "imagecontentmanager.h"

// TODO custom emojis

AllImageContentModel::AllImageContentModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // TODO connect to custom emojis changing;
}

QVariant AllImageContentModel::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    for (const auto &category : ImageContentManager::instance().emojis()) {
        if (row >= category.size()) {
            row -= category.size();
            continue;
        }
        if (role == ImageContentRole::DisplayNameRole) {
            return category[row].displayName;
        }
        if (role == ImageContentRole::EmojiRole) {
            return category[row].text;
        }
        if (role == ImageContentRole::IsStickerRole) {
            return false;
        }
        if (role == ImageContentRole::IsEmojiRole) {
            return true;
        }
    }
    return {};
}

int AllImageContentModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    auto sum = 0;
    for (const auto &category : ImageContentManager::instance().emojis()) {
        sum += category.size();
    }
    return sum;
}

QHash<int, QByteArray> AllImageContentModel::roleNames() const
{
    return {
        {ImageContentRole::DisplayNameRole, "displayName"},
        {ImageContentRole::EmojiRole, "text"},
    };
}
