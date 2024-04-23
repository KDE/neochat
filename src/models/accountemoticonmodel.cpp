// SPDX-FileCopyrightText: 2021-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "accountemoticonmodel.h"

#include <QImage>
#include <QMimeDatabase>

#include <Quotient/csapi/content-repo.h>
#include <Quotient/events/eventcontent.h>
#include <qcoro/qcorosignal.h>

#include "neochatconnection.h"

using namespace Quotient;

AccountEmoticonModel::AccountEmoticonModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int AccountEmoticonModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    if (!m_images) {
        return 0;
    }
    return m_images->images.size();
}

QVariant AccountEmoticonModel::data(const QModelIndex &index, int role) const
{
    if (m_connection == nullptr) {
        return {};
    }

    const auto &row = index.row();
    const auto &image = m_images->images[row];
    if (role == UrlRole) {
        return m_connection->makeMediaUrl(image.url).toString();
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

QHash<int, QByteArray> AccountEmoticonModel::roleNames() const
{
    return {
        {AccountEmoticonModel::UrlRole, "url"},
        {AccountEmoticonModel::BodyRole, "body"},
        {AccountEmoticonModel::ShortCodeRole, "shortcode"},
        {AccountEmoticonModel::IsStickerRole, "isSticker"},
        {AccountEmoticonModel::IsEmojiRole, "isEmoji"},
    };
}

NeoChatConnection *AccountEmoticonModel::connection() const
{
    return m_connection;
}

void AccountEmoticonModel::setConnection(NeoChatConnection *connection)
{
    if (m_connection) {
        disconnect(m_connection, nullptr, this, nullptr);
    }

    m_connection = connection;
    Q_EMIT connectionChanged();
    connect(m_connection, &Connection::accountDataChanged, this, [this](QString type) {
        if (type == QStringLiteral("im.ponies.user_emotes")) {
            reloadEmoticons();
        }
    });
    reloadEmoticons();
}

void AccountEmoticonModel::reloadEmoticons()
{
    if (m_connection == nullptr) {
        return;
    }

    QJsonObject json;
    if (m_connection->hasAccountData("im.ponies.user_emotes"_ls)) {
        json = m_connection->accountData("im.ponies.user_emotes"_ls)->contentJson();
    }
    const auto &content = ImagePackEventContent(json);
    beginResetModel();
    m_images = content;
    endResetModel();
}

void AccountEmoticonModel::deleteEmoticon(int index)
{
    if (m_connection == nullptr) {
        return;
    }

    QJsonObject data;
    m_images->images.removeAt(index);
    m_images->fillJson(&data);
    m_connection->setAccountData("im.ponies.user_emotes"_ls, data);
}

void AccountEmoticonModel::setEmoticonBody(int index, const QString &text)
{
    if (m_connection == nullptr) {
        return;
    }

    m_images->images[index].body = text;
    QJsonObject data;
    m_images->fillJson(&data);
    m_connection->setAccountData("im.ponies.user_emotes"_ls, data);
}

void AccountEmoticonModel::setEmoticonShortcode(int index, const QString &shortcode)
{
    if (m_connection == nullptr) {
        return;
    }

    m_images->images[index].shortcode = shortcode;
    QJsonObject data;
    m_images->fillJson(&data);
    m_connection->setAccountData("im.ponies.user_emotes"_ls, data);
}

void AccountEmoticonModel::setEmoticonImage(int index, const QUrl &source)
{
    if (m_connection == nullptr) {
        return;
    }
    doSetEmoticonImage(index, source);
}

QCoro::Task<void> AccountEmoticonModel::doSetEmoticonImage(int index, QUrl source)
{
    auto job = m_connection->uploadFile(source.isLocalFile() ? source.toLocalFile() : source.toString());
    co_await qCoro(job, &BaseJob::finished);
    if (job->error() != BaseJob::NoError) {
        co_return;
    }
    m_images->images[index].url = job->contentUri();
    auto mime = QMimeDatabase().mimeTypeForUrl(source);
    source.setScheme("file"_ls);
    QFileInfo fileInfo(source.isLocalFile() ? source.toLocalFile() : source.toString());
    EventContent::ImageInfo info;
    if (mime.name().startsWith("image/"_ls)) {
        QImage image(source.toLocalFile());
        info = EventContent::ImageInfo(source, fileInfo.size(), mime, image.size(), fileInfo.fileName());
    }
    m_images->images[index].info = info;
    QJsonObject data;
    m_images->fillJson(&data);
    m_connection->setAccountData("im.ponies.user_emotes"_ls, data);
}

QCoro::Task<void> AccountEmoticonModel::doAddEmoticon(QUrl source, QString shortcode, QString description, QString type)
{
    auto job = m_connection->uploadFile(source.isLocalFile() ? source.toLocalFile() : source.toString());
    co_await qCoro(job, &BaseJob::finished);
    if (job->error() != BaseJob::NoError) {
        co_return;
    }

    auto mime = QMimeDatabase().mimeTypeForUrl(source);
    source.setScheme("file"_ls);
    QFileInfo fileInfo(source.isLocalFile() ? source.toLocalFile() : source.toString());
    EventContent::ImageInfo info;
    if (mime.name().startsWith("image/"_ls)) {
        QImage image(source.toLocalFile());
        info = EventContent::ImageInfo(source, fileInfo.size(), mime, image.size(), fileInfo.fileName());
    }

    m_images->images.append(ImagePackEventContent::ImagePackImage{
        shortcode,
        job->contentUri(),
        description,
        info,
        QStringList{type},
    });
    QJsonObject data;
    m_images->fillJson(&data);
    m_connection->setAccountData("im.ponies.user_emotes"_ls, data);
}

void AccountEmoticonModel::addEmoticon(const QUrl &source, const QString &shortcode, const QString &description, const QString &type)
{
    if (m_connection == nullptr) {
        return;
    }
    doAddEmoticon(source, shortcode, description, type);
}

#include "moc_accountemoticonmodel.cpp"
