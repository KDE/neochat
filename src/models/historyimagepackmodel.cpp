// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "historyimagepackmodel.h"

#include <KLocalizedString>

#include "imagecontentmanager.h"

QVariant HistoryImagePackModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index);
    if (role == ImageContentPackRole::DisplayNameRole) {
        return i18n("History");
    }
    if (role == ImageContentPackRole::IconRole) {
        return QStringLiteral("⌛");
    }
    if (role == ImageContentPackRole::IdentifierRole) {
        return QStringLiteral("history");
    }
    if (role == ImageContentPackRole::IsStickerRole) {
        return true;
    }
    if (role == ImageContentPackRole::IsEmojiRole) {
        return true;
    }
    if (role == ImageContentPackRole::IsEmptyRole) {
        //TODO listen?
        return imageContentManager.recentEmojis().size() == 0;
    }
    return {};
}

int HistoryImagePackModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return 1;
}

QHash<int, QByteArray> HistoryImagePackModel::roleNames() const
{
    return {
        {ImageContentPackRole::DisplayNameRole, "displayName"},
        {ImageContentPackRole::IconRole, "emoji"},
        {ImageContentPackRole::IdentifierRole, "identifier"},
    };
}

HistoryImagePackModel::HistoryImagePackModel(QObject *parent)
    : QAbstractListModel(parent)
{
}
