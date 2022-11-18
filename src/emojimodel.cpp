// SPDX-FileCopyrightText: 2017 Konstantinos Sideris <siderisk@auth.gr>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QVariant>

#include "emojimodel.h"
#include <QDebug>

#include <algorithm>

#include <KLocalizedString>
#include <qnamespace.h>

EmojiModel::EmojiModel(QObject *parent)
    : QAbstractListModel(parent)
{
    if (_emojis.isEmpty()) {
#include "emojis.h"
    }
}

int EmojiModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    int total = 0;
    for (const auto &category : _emojis) {
        total += category.count();
    }
    return total;
}

QVariant EmojiModel::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    for (const auto &category : _emojis) {
        if (row >= category.count()) {
            row -= category.count();
            continue;
        }
        auto emoji = category[row].value<Emoji>();
        switch (role) {
        case TextRole:
            return emoji.shortName;
        case UnicodeRole:
            return emoji.unicode;
        }
    }
    return {};
}

QHash<int, QByteArray> EmojiModel::roleNames() const
{
    return {{TextRole, "text"}, {UnicodeRole, "unicode"}};
}

QMultiHash<QString, QVariant> EmojiModel::_tones = {
#include "emojitones.h"
};

QVariantList EmojiModel::history() const
{
    return m_settings.value("Editor/emojis", QVariantList()).toList();
}

QVariantList EmojiModel::filterModel(const QString &filter, bool limit)
{
    QVariantList result;

    for (const auto &e : _emojis.values()) {
        for (const auto &variant : e) {
            const auto &emoji = qvariant_cast<Emoji>(variant);
            if (emoji.shortName.contains(filter, Qt::CaseInsensitive)) {
                result.append(variant);
                if (result.length() > 10 && limit) {
                    return result;
                }
            }
        }
    }

    return result;
}

void EmojiModel::emojiUsed(const QVariant &modelData)
{
    QVariantList list = history();

    auto it = list.begin();
    while (it != list.end()) {
        if ((*it).value<Emoji>().unicode == modelData.value<Emoji>().unicode) {
            it = list.erase(it);
        } else {
            it++;
        }
    }

    list.push_front(modelData);
    m_settings.setValue("Editor/emojis", list);

    Q_EMIT historyChanged();
}

QVariantList EmojiModel::emojis(Category category) const
{
    if (category == History) {
        return history();
    }
    return _emojis[category];
}

QVariantList EmojiModel::tones(const QString &baseEmoji) const
{
    return _tones.values(baseEmoji);
}

QHash<EmojiModel::Category, QVariantList> EmojiModel::_emojis;

QVariantList EmojiModel::categories() const
{
    return QVariantList{
        // {QVariantMap{
        //     {"category", EmojiModel::Search},
        //     {"name", i18nc("Search for emojis", "Search")},
        //     {"emoji", QStringLiteral("🔎")},
        // }},
        {QVariantMap{
            {"category", EmojiModel::History},
            {"name", i18nc("Previously used emojis", "History")},
            {"emoji", QStringLiteral("⌛️")},
        }},
        {QVariantMap{
            {"category", EmojiModel::Custom},
            {"name", i18nc("'Custom' is a category of emoji", "Custom")},
            {"emoji", QStringLiteral("😏")},
        }},
        {QVariantMap{
            {"category", EmojiModel::Smileys},
            {"name", i18nc("'Smileys' is a category of emoji", "Smileys")},
            {"emoji", QStringLiteral("😏")},
        }},
        {QVariantMap{
            {"category", EmojiModel::People},
            {"name", i18nc("'People' is a category of emoji", "People")},
            {"emoji", QStringLiteral("🙋‍♂️")},
        }},
        {QVariantMap{
            {"category", EmojiModel::Nature},
            {"name", i18nc("'Nature' is a category of emoji", "Nature")},
            {"emoji", QStringLiteral("🌲")},
        }},
        {QVariantMap{
            {"category", EmojiModel::Food},
            {"name", i18nc("'Food' is a category of emoji", "Food")},
            {"emoji", QStringLiteral("🍛")},
        }},
        {QVariantMap{
            {"category", EmojiModel::Activities},
            {"name", i18nc("'Activities' is a category of emoji", "Activities")},
            {"emoji", QStringLiteral("🚁")},
        }},
        {QVariantMap{
            {"category", EmojiModel::Travel},
            {"name", i18nc("'Travel' is  a category of emoji", "Travel")},
            {"emoji", QStringLiteral("🚅")},
        }},
        {QVariantMap{
            {"category", EmojiModel::Objects},
            {"name", i18nc("'Objects' is a category of emoji", "Objects")},
            {"emoji", QStringLiteral("💡")},
        }},
        {QVariantMap{
            {"category", EmojiModel::Symbols},
            {"name", i18nc("'Symbols' is a category of emoji", "Symbols")},
            {"emoji", QStringLiteral("🔣")},
        }},
        {QVariantMap{
            {"category", EmojiModel::Flags},
            {"name", i18nc("'Flags' is a category of emoji", "Flags")},
            {"emoji", QStringLiteral("🏁")},
        }},
    };
}
