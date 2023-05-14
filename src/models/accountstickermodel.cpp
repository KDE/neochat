// SPDX-FileCopyrightText: 2021-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "accountstickermodel.h"

#include <csapi/content-repo.h>
#include <qcoro/qcorosignal.h>

using namespace Quotient;

AccountStickerModel::AccountStickerModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int AccountStickerModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    if (!m_images) {
        return 0;
    }
    return m_images->images.size();
}

QVariant AccountStickerModel::data(const QModelIndex &index, int role) const
{
    const auto &row = index.row();
    const auto &image = m_images->images[row];
    if (role == UrlRole) {
#ifdef QUOTIENT_07
        return m_connection->makeMediaUrl(image.url);
#else
        return QUrl();
#endif
    }
    if (role == BodyRole) {
        if (image.body) {
            return *image.body;
        }
    }
    if (role == ShortCodeRole) {
        return image.shortcode;
    }
    if (role == IsStickerRole) {
        if (image.usage) {
            return image.usage->isEmpty() || image.usage->contains("sticker"_ls);
        }
        if (m_images->pack && m_images->pack->usage) {
            return m_images->pack->usage->isEmpty() || m_images->pack->usage->contains("sticker"_ls);
        }
        return true;
    }
    if (role == IsEmojiRole) {
        if (image.usage) {
            return image.usage->isEmpty() || image.usage->contains("emoticon"_ls);
        }
        if (m_images->pack && m_images->pack->usage) {
            return m_images->pack->usage->isEmpty() || m_images->pack->usage->contains("emoticon"_ls);
        }
        return true;
    }
    return {};
}

QHash<int, QByteArray> AccountStickerModel::roleNames() const
{
    return {
        {AccountStickerModel::UrlRole, "url"},
        {AccountStickerModel::BodyRole, "body"},
        {AccountStickerModel::ShortCodeRole, "shortcode"},
        {AccountStickerModel::IsStickerRole, "isSticker"},
        {AccountStickerModel::IsEmojiRole, "isEmoji"},
    };
}

Connection *AccountStickerModel::connection() const
{
    return m_connection;
}

void AccountStickerModel::setConnection(Connection *connection)
{
    if (m_connection) {
        disconnect(m_connection, nullptr, this, nullptr);
    }

    m_connection = connection;
    Q_EMIT connectionChanged();
    connect(m_connection, &Connection::accountDataChanged, this, [this](QString type) {
        if (type == QStringLiteral("im.ponies.user_emotes")) {
            reloadStickers();
        }
    });
    reloadStickers();
}

void AccountStickerModel::reloadStickers()
{
    if (!m_connection->hasAccountData("im.ponies.user_emotes"_ls)) {
        return;
    }
    auto json = m_connection->accountData("im.ponies.user_emotes"_ls)->contentJson();
    const auto &content = ImagePackEventContent(json);
    beginResetModel();
    m_images = content;
    endResetModel();
}

void AccountStickerModel::deleteSticker(int index)
{
    QJsonObject data;
    m_images->images.removeAt(index);
    m_images->fillJson(&data);
    m_connection->setAccountData("im.ponies.user_emotes"_ls, data);
}

void AccountStickerModel::setStickerBody(int index, const QString &text)
{
    m_images->images[index].body = text;
    QJsonObject data;
    m_images->fillJson(&data);
    m_connection->setAccountData("im.ponies.user_emotes"_ls, data);
}

void AccountStickerModel::setStickerShortcode(int index, const QString &shortcode)
{
    m_images->images[index].shortcode = shortcode;
    QJsonObject data;
    m_images->fillJson(&data);
    m_connection->setAccountData("im.ponies.user_emotes"_ls, data);
}

void AccountStickerModel::setStickerImage(int index, const QUrl &source)
{
    doSetStickerImage(index, source);
}

QCoro::Task<void> AccountStickerModel::doSetStickerImage(int index, QUrl source)
{
    auto job = m_connection->uploadFile(source.isLocalFile() ? source.toLocalFile() : source.toString());
    co_await qCoro(job, &BaseJob::finished);
    if (job->error() != BaseJob::NoError) {
        co_return;
    }
#ifdef QUOTIENT_07
    m_images->images[index].url = job->contentUri().toString();
#else
    m_images->images[index].url = job->contentUri();
#endif
    m_images->images[index].info = none;
    QJsonObject data;
    m_images->fillJson(&data);
    m_connection->setAccountData("im.ponies.user_emotes"_ls, data);
}

QCoro::Task<void> AccountStickerModel::doAddSticker(QUrl source, QString shortcode, QString description)
{
    auto job = m_connection->uploadFile(source.isLocalFile() ? source.toLocalFile() : source.toString());
    co_await qCoro(job, &BaseJob::finished);
    if (job->error() != BaseJob::NoError) {
        co_return;
    }
    m_images->images.append(ImagePackEventContent::ImagePackImage{
        shortcode,
        job->contentUri(),
        description,
        none,
        QStringList{"sticker"_ls},
    });
    QJsonObject data;
    m_images->fillJson(&data);
    m_connection->setAccountData("im.ponies.user_emotes"_ls, data);
}

void AccountStickerModel::addSticker(const QUrl &source, const QString &shortcode, const QString &description)
{
    doAddSticker(source, shortcode, description);
}
