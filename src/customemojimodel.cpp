// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "emojimodel.h"
#include "customemojimodel_p.h"

enum Roles {
    Name,
    ImageURL,
    ModelData, // for emulating the regular emoji model's usage, otherwise the UI code would get too complicated
};

CustomEmojiModel::CustomEmojiModel(QObject* parent) : QAbstractListModel(parent), d(new Private)
{
    connect(this, &CustomEmojiModel::connectionChanged, this, &CustomEmojiModel::fetchEmojies);
    connect(this, &CustomEmojiModel::connectionChanged, this, [this]() {
        if (!d->conn) return;

        connect(d->conn, &Connection::accountDataChanged, this, [this](const QString& id) {
            if (id != QStringLiteral("im.ponies.user_emotes")) {
                return;
            }
            fetchEmojies();
        });
    });
}

CustomEmojiModel::~CustomEmojiModel()
{

}

QVariant CustomEmojiModel::data(const QModelIndex& idx, int role) const
{
    const auto row = idx.row();
    if (row >= d->emojies.length()) {
        return QVariant();
    }
    const auto& data = d->emojies[row];

    switch (Roles(role)) {
    case Roles::ModelData:
        return QVariant::fromValue(Emoji(
            QStringLiteral("image://mxc/") + data.url.mid(6),
            data.name,
            true
        ));
    case Roles::Name:
        return data.name;
    case Roles::ImageURL:
        return QUrl(QStringLiteral("image://mxc/") + data.url.mid(6));
    }

    return QVariant();
}

int CustomEmojiModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return d->emojies.length();
}

QHash<int,QByteArray> CustomEmojiModel::roleNames() const
{
    return {
        { Name, "name" },
        { ImageURL, "imageURL" },
        { ModelData, "modelData" },
    };
}

Connection* CustomEmojiModel::connection() const
{
    return d->conn;
}

void CustomEmojiModel::setConnection(Connection* it)
{
    if (d->conn == it) {
        return;
    }
    if (d->conn != nullptr) {
        disconnect(d->conn, nullptr, this, nullptr);
    }
    d->conn = it;
    Q_EMIT connectionChanged();
}

QString CustomEmojiModel::preprocessText(const QString &it)
{
    auto cp = it;
    for (const auto& emoji : qAsConst(d->emojies)) {
        cp.replace(emoji.regexp, QStringLiteral(R"(<img data-mx-emoticon="" src="%1" alt="%2" title="%2" height="32" vertical-align="middle" />)").arg(emoji.url).arg(emoji.name));
    }
    return cp;
}

QVariantList CustomEmojiModel::filterModel(const QString &filter)
{
    QVariantList results;
    for (const auto& emoji : qAsConst(d->emojies)) {
        if (results.length() >= 10) break;
        if (!emoji.name.contains(filter, Qt::CaseInsensitive)) continue;

        results << QVariant::fromValue(Emoji(
            QStringLiteral("image://mxc/") + emoji.url.mid(6),
            emoji.name,
            true
        ));
    }
    return results;
}
