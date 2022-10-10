// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "customemojimodel.h"
#include "controller.h"
#include "emojimodel.h"

#include <connection.h>

using namespace Quotient;

CustomEmojiModel::CustomEmojiModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&Controller::instance(), &Controller::activeConnectionChanged, this, [this]() {
        if (!Controller::instance().activeConnection()) {
            return;
        }
        CustomEmojiModel::fetchEmojis();
        disconnect(nullptr, &Connection::accountDataChanged, this, nullptr);
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
        return data.name;
    case Roles::ImageURL:
        return QUrl(QStringLiteral("image://mxc/") + data.url.mid(6));
    case Roles::MxcUrl:
        return data.url.mid(6);
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
    auto handledText = text;
    for (const auto &emoji : std::as_const(m_emojis)) {
        handledText.replace(
            emoji.regexp,
            QStringLiteral(R"(<img data-mx-emoticon="" src="%1" alt="%2" title="%2" height="32" vertical-align="middle" />)").arg(emoji.url, emoji.name));
    }
    return handledText;
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
