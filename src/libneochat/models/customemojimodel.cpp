// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "customemojimodel.h"

#include <QImage>
#include <QMimeDatabase>

#include "emojimodel.h"

#include <Quotient/csapi/account-data.h>
#include <Quotient/csapi/content-repo.h>

using namespace Quotient;

void CustomEmojiModel::setConnection(NeoChatConnection *connection)
{
    if (connection == m_connection) {
        return;
    }
    m_connection = connection;
    Q_EMIT connectionChanged();
    fetchEmojis();
}

NeoChatConnection *CustomEmojiModel::connection() const
{
    return m_connection;
}

void CustomEmojiModel::fetchEmojis()
{
    if (!m_connection) {
        return;
    }

    const auto &data = m_connection->accountData("im.ponies.user_emotes"_L1);
    if (data == nullptr) {
        return;
    }
    QJsonObject emojis = data->contentJson()["images"_L1].toObject();

    // TODO: Remove with stable migration
    const auto legacyEmojis = data->contentJson()["emoticons"_L1].toObject();
    for (const auto &emoji : legacyEmojis.keys()) {
        if (!emojis.contains(emoji)) {
            emojis[emoji] = legacyEmojis[emoji];
        }
    }

    beginResetModel();
    m_emojis.clear();

    for (const auto &emoji : emojis.keys()) {
        const auto &data = emojis[emoji];

        const auto e = emoji.startsWith(":"_L1) ? emoji : (u":"_s + emoji + u":"_s);

        m_emojis << CustomEmoji{e, data.toObject()["url"_L1].toString(), QRegularExpression(e)};
    }

    endResetModel();
}

void CustomEmojiModel::addEmoji(const QString &name, const QUrl &location)
{
    using namespace Quotient;

    m_connection->uploadFile(location.toLocalFile()).onResult([name, location, this](const auto &job) {
        const auto &data = m_connection->accountData("im.ponies.user_emotes"_L1);
        auto json = data != nullptr ? data->contentJson() : QJsonObject();
        auto emojiData = json["images"_L1].toObject();

        QString url;
        url = job->contentUri().toString();

        QImage image(location.toLocalFile());
        QJsonObject imageInfo;
        imageInfo["w"_L1] = image.width();
        imageInfo["h"_L1] = image.height();
        imageInfo["mimetype"_L1] = QMimeDatabase().mimeTypeForFile(location.toLocalFile()).name();
        imageInfo["size"_L1] = image.sizeInBytes();

        emojiData["%1"_L1.arg(name)] = QJsonObject({
            {u"url"_s, url},
            {u"info"_s, imageInfo},
            {u"body"_s, location.fileName()},
            {u"usage"_s, "emoticon"_L1},
        });

        json["images"_L1] = emojiData;
        m_connection->setAccountData("im.ponies.user_emotes"_L1, json);
    });
}

void CustomEmojiModel::removeEmoji(const QString &name)
{
    using namespace Quotient;

    const auto &data = m_connection->accountData("im.ponies.user_emotes"_L1);
    Q_ASSERT(data);
    auto json = data->contentJson();
    const QString _name = name.mid(1).chopped(1);
    auto emojiData = json["images"_L1].toObject();

    if (emojiData.contains(name)) {
        emojiData.remove(name);
        json["images"_L1] = emojiData;
    }
    if (emojiData.contains(_name)) {
        emojiData.remove(_name);
        json["images"_L1] = emojiData;
    }
    emojiData = json["emoticons"_L1].toObject();
    if (emojiData.contains(name)) {
        emojiData.remove(name);
        json["emoticons"_L1] = emojiData;
    }
    if (emojiData.contains(_name)) {
        emojiData.remove(_name);
        json["emoticons"_L1] = emojiData;
    }
    m_connection->setAccountData("im.ponies.user_emotes"_L1, json);
}

CustomEmojiModel::CustomEmojiModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &CustomEmojiModel::connectionChanged, this, [this]() {
        if (!m_connection) {
            return;
        }
        CustomEmojiModel::fetchEmojis();
        connect(m_connection, &Connection::accountDataChanged, this, [this](const QString &id) {
            if (id != u"im.ponies.user_emotes"_s) {
                return;
            }
            fetchEmojis();
        });
    });
    CustomEmojiModel::fetchEmojis();
}

QVariant CustomEmojiModel::data(const QModelIndex &idx, int role) const
{
    const auto row = idx.row();
    if (row >= m_emojis.length()) {
        return QVariant();
    }
    const auto &data = m_emojis[row];

    switch (Roles(role)) {
    case Roles::ModelData:
        return QVariant::fromValue(Emoji(m_connection->makeMediaUrl(QUrl(data.url)).toString(), data.name, true));
    case Roles::Name:
    case Roles::DisplayRole:
    case Roles::ReplacedTextRole:
        return data.name;
    case Roles::ImageURL:
        return m_connection->makeMediaUrl(QUrl(data.url));
    case Roles::MxcUrl:
        return m_connection->makeMediaUrl(QUrl(data.url));
    default:
        return {};
    }

    return QVariant();
}

int CustomEmojiModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_emojis.length();
}

QHash<int, QByteArray> CustomEmojiModel::roleNames() const
{
    return {
        {Name, "name"},
        {ImageURL, "imageURL"},
        {ModelData, "modelData"},
        {MxcUrl, "mxcUrl"},
    };
}

QString CustomEmojiModel::preprocessText(QString text)
{
    for (const auto &emoji : std::as_const(m_emojis)) {
        text.replace(emoji.regexp,
                     uR"(<img data-mx-emoticon="" src="%1" alt="%2" title="%2" height="32" vertical-align="middle" />)"_s.arg(emoji.url, emoji.name));
    }
    return text;
}

QVariantList CustomEmojiModel::filterModel(const QString &filter)
{
    QVariantList results;
    for (const auto &emoji : std::as_const(m_emojis)) {
        if (results.length() >= 10)
            break;
        if (!emoji.name.contains(filter, Qt::CaseInsensitive))
            continue;

        results << QVariant::fromValue(Emoji(m_connection->makeMediaUrl(QUrl(emoji.url)).toString(), emoji.name, true));
    }
    return results;
}

#include "moc_customemojimodel.cpp"
