// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "customemojimodel.h"

#include <QImage>
#include <QMimeDatabase>

#include "controller.h"
#include "emojimodel.h"

#include <connection.h>
#include <csapi/account-data.h>
#include <csapi/content-repo.h>
#include <events/eventcontent.h>

using namespace Quotient;

void CustomEmojiModel::fetchEmojis()
{
    if (!Controller::instance().activeConnection()) {
        return;
    }

    const auto &data = Controller::instance().activeConnection()->accountData("im.ponies.user_emotes");
    if (data == nullptr) {
        return;
    }
    QJsonObject emojis = data->contentJson()["images"].toObject();

    // TODO: Remove with stable migration
    const auto legacyEmojis = data->contentJson()["emoticons"].toObject();
    for (const auto &emoji : legacyEmojis.keys()) {
        if (!emojis.contains(emoji)) {
            emojis[emoji] = legacyEmojis[emoji];
        }
    }

    beginResetModel();
    m_emojis.clear();

    for (const auto &emoji : emojis.keys()) {
        const auto &data = emojis[emoji];

        const auto e = emoji.startsWith(":") ? emoji : (QStringLiteral(":") + emoji + QStringLiteral(":"));

        m_emojis << CustomEmoji{e, data.toObject()["url"].toString(), QRegularExpression(e)};
    }

    endResetModel();
}

void CustomEmojiModel::addEmoji(const QString &name, const QUrl &location)
{
    using namespace Quotient;

    auto job = Controller::instance().activeConnection()->uploadFile(location.toLocalFile());

    connect(job, &BaseJob::success, this, [name, location, job] {
        const auto &data = Controller::instance().activeConnection()->accountData("im.ponies.user_emotes");
        auto json = data != nullptr ? data->contentJson() : QJsonObject();
        auto emojiData = json["images"].toObject();

        QString url;
        url = job->contentUri().toString();

        QImage image(location.toLocalFile());
        QJsonObject imageInfo;
        imageInfo["w"] = image.width();
        imageInfo["h"] = image.height();
        imageInfo["mimetype"] = QMimeDatabase().mimeTypeForFile(location.toLocalFile()).name();
        imageInfo["size"] = image.sizeInBytes();

        emojiData[QStringLiteral("%1").arg(name)] = QJsonObject({
            {QStringLiteral("url"), url},
            {QStringLiteral("info"), imageInfo},
            {QStringLiteral("body"), location.fileName()},
            {"usage"_ls, "emoticon"_ls},
        });

        json["images"] = emojiData;
        Controller::instance().activeConnection()->setAccountData("im.ponies.user_emotes", json);
    });
}

void CustomEmojiModel::removeEmoji(const QString &name)
{
    using namespace Quotient;

    const auto &data = Controller::instance().activeConnection()->accountData("im.ponies.user_emotes");
    Q_ASSERT(data);
    auto json = data->contentJson();
    const QString _name = name.mid(1).chopped(1);
    auto emojiData = json["images"].toObject();

    if (emojiData.contains(name)) {
        emojiData.remove(name);
        json["images"] = emojiData;
    }
    if (emojiData.contains(_name)) {
        emojiData.remove(_name);
        json["images"] = emojiData;
    }
    emojiData = json["emoticons"].toObject();
    if (emojiData.contains(name)) {
        emojiData.remove(name);
        json["emoticons"] = emojiData;
    }
    if (emojiData.contains(_name)) {
        emojiData.remove(_name);
        json["emoticons"] = emojiData;
    }
    Controller::instance().activeConnection()->setAccountData("im.ponies.user_emotes", json);
}

CustomEmojiModel::CustomEmojiModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&Controller::instance(), &Controller::activeConnectionChanged, this, [this]() {
        if (!Controller::instance().activeConnection()) {
            return;
        }
        CustomEmojiModel::fetchEmojis();
        connect(Controller::instance().activeConnection(), &Connection::accountDataChanged, this, [this](const QString &id) {
            if (id != QStringLiteral("im.ponies.user_emotes")) {
                return;
            }
            fetchEmojis();
        });
    });
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
        return QVariant::fromValue(Emoji(QStringLiteral("image://mxc/") + data.url.mid(6), data.name, true));
    case Roles::Name:
    case Roles::DisplayRole:
    case Roles::ReplacedTextRole:
        return data.name;
    case Roles::ImageURL:
        return QUrl(QStringLiteral("image://mxc/") + data.url.mid(6));
    case Roles::MxcUrl:
        return data.url.mid(6);
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

QString CustomEmojiModel::preprocessText(const QString &text)
{
    auto parts = text.split("```");
    bool skip = true;
    for (auto &part : parts) {
        skip = !skip;
        if (skip) {
            continue;
        }
        for (const auto &emoji : std::as_const(m_emojis)) {
            part.replace(
                emoji.regexp,
                QStringLiteral(R"(<img data-mx-emoticon="" src="%1" alt="%2" title="%2" height="32" vertical-align="middle" />)").arg(emoji.url, emoji.name));
        }
    }
    return parts.join("```");
}

QVariantList CustomEmojiModel::filterModel(const QString &filter)
{
    QVariantList results;
    for (const auto &emoji : std::as_const(m_emojis)) {
        if (results.length() >= 10)
            break;
        if (!emoji.name.contains(filter, Qt::CaseInsensitive))
            continue;

        results << QVariant::fromValue(Emoji(QStringLiteral("image://mxc/") + emoji.url.mid(6), emoji.name, true));
    }
    return results;
}

#include "moc_customemojimodel.cpp"
