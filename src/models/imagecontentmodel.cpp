// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imagecontentmodel.h"

#include <QDebug>

#include <Quotient/connection.h>

#include "controller.h"
#include "imagecontentmanager.h"

ImageContentModel::ImageContentModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ImageContentModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_category == QStringLiteral("account")) {
        return imageContentManager.accountImages().size();
    }
    if (m_category.contains(u'@')) {
        return imageContentManager.roomImages()[{m_roomId, m_stateKey}].size();
    }
    return imageContentManager.emojis()[m_category].count();
}

QVariant ImageContentModel::emojiData(int row, int role) const
{
    const auto emoji = imageContentManager.emojis()[m_category][row];
    if (role == ImageContentRole::DisplayNameRole) {
        return emoji.displayName;
    }
    if (role == ImageContentRole::EmojiRole) {
        return emoji.text;
    }
    if (role == ImageContentRole::IsCustomRole) {
        return false;
    }
    if (role == ImageContentRole::IsEmojiRole) {
        return true;
    }
    if (role == ImageContentRole::IsStickerRole) {
        return false;
    }
    if (role == ImageContentRole::HasTonesRole) {
        return true; // TODO
    }
    return {};
}

QVariant ImageContentModel::accountData(int row, int role) const
{
    const auto &image = imageContentManager.accountImages()[row];
    if (role == ImageContentRole::DisplayNameRole) {
        return image.shortcode;
    }
    if (role == ImageContentRole::EmojiRole) {
        return QStringLiteral("<img src=\"%1\" height=\"32\" width=\"32\"/>")
            .arg(Controller::instance().activeConnection()->makeMediaUrl(image.url).toString());
    }
    if (role == ImageContentRole::ShortCodeRole) {
        return image.shortcode;
    }
    if (role == ImageContentRole::IsCustomRole) {
        return true;
    }
    if (role == ImageContentRole::IsEmojiRole) {
        return !image.usage || image.usage->isEmpty() || image.usage->contains(QStringLiteral("emoticon"));
    }
    if (role == ImageContentRole::IsStickerRole) {
        return !image.usage || image.usage->isEmpty() || image.usage->contains(QStringLiteral("sticker"));
    }
    return {};
}

QVariant ImageContentModel::roomData(int row, int role) const
{
    const auto image = imageContentManager.roomImages()[{m_roomId, m_stateKey}][row];
    if (role == ImageContentRole::DisplayNameRole) {
        return image.shortcode;
    }
    if (role == ImageContentRole::EmojiRole) {
        return QStringLiteral("<img src=\"%1\" height=\"32\" width=\"32\"/>")
            .arg(Controller::instance().activeConnection()->makeMediaUrl(image.url).toString());
    }
    if (role == ImageContentRole::ShortCodeRole) {
        return image.shortcode;
    }
    if (role == ImageContentRole::IsCustomRole) {
        return true;
    }
    if (role == ImageContentRole::IsEmojiRole) {
        return true; // For room image packs, we're ignoring the usage of the individual images.
    }
    if (role == ImageContentRole::IsStickerRole) {
        return true;
    }
    return {};
}

QVariant ImageContentModel::data(const QModelIndex &index, int role) const
{
    const auto &row = index.row();
    if (m_category == QStringLiteral("account")) {
        return accountData(row, role);
    }
    if (m_category.contains(u'@')) {
        return roomData(row, role);
    }
    return emojiData(row, role);
}

QHash<int, QByteArray> ImageContentModel::roleNames() const
{
    return {
        {ImageContentRole::DisplayNameRole, "displayName"},
        {ImageContentRole::EmojiRole, "text"},
        {ImageContentRole::ShortCodeRole, "shortCode"},
        {ImageContentRole::IsCustomRole, "isCustom"},
        {ImageContentRole::IsStickerRole, "isSticker"},
        {ImageContentRole::IsEmojiRole, "isEmoji"},
        {ImageContentRole::HasTonesRole, "hasTones"},
    };
}

QString ImageContentModel::category() const
{
    return m_category;
}

void ImageContentModel::setCategory(const QString &category)
{
    if (category == m_category) {
        return;
    }

    beginResetModel();
    m_category = category;
    if (m_category.contains(u'@')) {
        const auto &split = m_category.split(u'@');
        m_roomId = split[0];
        m_stateKey = split[1];
    } else {
        m_roomId = QString();
        m_stateKey = QString();
    }
    endResetModel();

    if (m_category == QStringLiteral("account")) {
        connect(&ImageContentManager::instance(), &ImageContentManager::accountImagesChanged, this, [this]() {
            beginResetModel();
            endResetModel();
        });
    } else {
        disconnect(&ImageContentManager::instance(), &ImageContentManager::accountImagesChanged, this, nullptr);
    }

    Q_EMIT categoryChanged();
}

#include "moc_imagecontentmodel.cpp"
