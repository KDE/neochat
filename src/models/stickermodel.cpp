// SPDX-FileCopyrightText: 2021-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "stickermodel.h"

#include "models/imagepacksmodel.h"

using namespace Quotient;

StickerModel::StickerModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int StickerModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return m_images.size();
}
QVariant StickerModel::data(const QModelIndex &index, int role) const
{
    const auto &row = index.row();
    const auto &image = m_images[row];
    if (role == UrlRole) {
#ifdef QUOTIENT_07
        return m_room->connection()->makeMediaUrl(image.url);
#endif
    }
    if (role == BodyRole) {
        if (image.body) {
            return *image.body;
        }
    }
    if (role == IsStickerRole) {
        if (image.usage) {
            return image.usage->isEmpty() || image.usage->contains("sticker"_ls);
        }
        return true;
    }
    if (role == IsEmojiRole) {
        if (image.usage) {
            return image.usage->isEmpty() || image.usage->contains("emoticon"_ls);
        }
        return true;
    }
    return {};
}

QHash<int, QByteArray> StickerModel::roleNames() const
{
    return {
        {UrlRole, "url"},
        {BodyRole, "body"},
        {IsStickerRole, "isSticker"},
        {IsEmojiRole, "isEmoji"},
    };
}
ImagePacksModel *StickerModel::model() const
{
    return m_model;
}

void StickerModel::setModel(ImagePacksModel *model)
{
    if (m_model) {
        disconnect(m_model, nullptr, this, nullptr);
    }
    connect(model, &ImagePacksModel::roomChanged, this, [this]() {
        reloadImages();
    });
    connect(model, &ImagePacksModel::imagesLoaded, this, &StickerModel::reloadImages);
    m_model = model;
    reloadImages();
    Q_EMIT modelChanged();
}

int StickerModel::packIndex() const
{
    return m_index;
}
void StickerModel::setPackIndex(int index)
{
    m_index = index;
    Q_EMIT packIndexChanged();
    reloadImages();
}

void StickerModel::reloadImages()
{
    beginResetModel();
    if (m_model) {
        m_images = m_model->images(m_index);
    }
    endResetModel();
}

NeoChatRoom *StickerModel::room() const
{
    return m_room;
}

void StickerModel::setRoom(NeoChatRoom *room)
{
    if (room) {
        disconnect(room->connection(), nullptr, this, nullptr);
    }
    m_room = room;
    Q_EMIT roomChanged();
}

void StickerModel::postSticker(int index)
{
    const auto &image = m_images[index];
    const auto &body = image.body ? *image.body : QString();
    QJsonObject infoJson;
    if (image.info) {
        infoJson["w"] = image.info->imageSize.width();
        infoJson["h"] = image.info->imageSize.height();
        infoJson["mimetype"] = image.info->mimeType.name();
        infoJson["size"] = image.info->payloadSize;
        // TODO thumbnail
    }
    QJsonObject content{
        {"body"_ls, body},
        {"url"_ls, image.url.toString()},
        {"info"_ls, infoJson},
    };
    m_room->postJson("m.sticker", content);
}

#include "moc_stickermodel.cpp"
