// SPDX-FileCopyrightText: 2017 Konstantinos Sideris <siderisk@auth.gr>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QVariant>

#include "emojimodel.h"
#include "emojitones.h"
#include <QDebug>

#include <algorithm>

#include "customemojimodel.h"
#include <KLocalizedString>

EmojiModel::EmojiModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_config(KSharedConfig::openStateConfig())
    , m_configGroup(KConfigGroup(m_config, QStringLiteral("Editor")))
{
    if (_emojis.isEmpty()) {
#include "emojis.h"
    }
}

int EmojiModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    int total = 0;
    for (const auto &category : std::as_const(_emojis)) {
        total += category.count();
    }
    return total;
}

QVariant EmojiModel::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    for (const auto &category : std::as_const(_emojis)) {
        if (row >= category.count()) {
            row -= category.count();
            continue;
        }
        auto emoji = category[row].value<Emoji>();
        switch (role) {
        case ShortNameRole:
            return QStringLiteral(":%1:").arg(emoji.shortName);
        case UnicodeRole:
        case ReplacedTextRole:
            return emoji.unicode;
        case InvalidRole:
            return QStringLiteral("invalid");
        case DisplayRole:
            return QStringLiteral("%2   :%1:").arg(emoji.shortName, emoji.unicode);
        case DescriptionRole:
            return emoji.description;
        }
    }
    return {};
}

QHash<int, QByteArray> EmojiModel::roleNames() const
{
    return {{ShortNameRole, "shortName"}, {UnicodeRole, "unicode"}};
}

QStringList EmojiModel::lastUsedEmojis() const
{
    return m_configGroup.readEntry(QStringLiteral("lastUsedEmojis"), QStringList());
}

QVariantList EmojiModel::filterModel(const QString &filter, bool limit)
{
    auto emojis = CustomEmojiModel::instance().filterModel(filter);
    emojis += filterModelNoCustom(filter, limit);
    return emojis;
}

QVariantList EmojiModel::filterModelNoCustom(const QString &filter, bool limit)
{
    QVariantList result;

    const auto &values = _emojis.values();
    for (const auto &e : values) {
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
    auto list = lastUsedEmojis();
    const auto emoji = modelData.value<Emoji>();

    auto it = list.begin();
    while (it != list.end()) {
        if (*it == emoji.shortName) {
            it = list.erase(it);
        } else {
            it++;
        }
    }

    list.push_front(emoji.shortName);

    m_configGroup.writeEntry(QStringLiteral("lastUsedEmojis"), list);

    Q_EMIT historyChanged();
}

QVariantList EmojiModel::emojis(Category category) const
{
    if (category == History) {
        return emojiHistory();
    }
    if (category == HistoryNoCustom) {
        QVariantList list;
        const auto &history = emojiHistory();
        for (const auto &e : history) {
            auto emoji = qvariant_cast<Emoji>(e);
            if (!emoji.isCustom) {
                list.append(e);
            }
        }
        return list;
    }
    if (category == Custom) {
        return CustomEmojiModel::instance().filterModel({});
    }
    return _emojis[category];
}

QVariantList EmojiModel::tones(const QString &baseEmoji) const
{
    if (baseEmoji.endsWith(QStringLiteral("tone"))) {
        return EmojiTones::_tones.values(baseEmoji.split(QStringLiteral(":"))[0]);
    }
    return EmojiTones::_tones.values(baseEmoji);
}

QHash<EmojiModel::Category, QVariantList> EmojiModel::_emojis;

QVariantList EmojiModel::categories() const
{
    return QVariantList{
        {QVariantMap{
            {QStringLiteral("category"), EmojiModel::HistoryNoCustom},
            {QStringLiteral("name"), i18nc("Previously used emojis", "History")},
            {QStringLiteral("emoji"), QStringLiteral("⌛️")},
        }},
        {QVariantMap{
            {QStringLiteral("category"), EmojiModel::Smileys},
            {QStringLiteral("name"), i18nc("'Smileys' is a category of emoji", "Smileys")},
            {QStringLiteral("emoji"), QStringLiteral("😏")},
        }},
        {QVariantMap{
            {QStringLiteral("category"), EmojiModel::People},
            {QStringLiteral("name"), i18nc("'People' is a category of emoji", "People")},
            {QStringLiteral("emoji"), QStringLiteral("🙋‍♂️")},
        }},
        {QVariantMap{
            {QStringLiteral("category"), EmojiModel::Nature},
            {QStringLiteral("name"), i18nc("'Nature' is a category of emoji", "Nature")},
            {QStringLiteral("emoji"), QStringLiteral("🌲")},
        }},
        {QVariantMap{
            {QStringLiteral("category"), EmojiModel::Food},
            {QStringLiteral("name"), i18nc("'Food' is a category of emoji", "Food")},
            {QStringLiteral("emoji"), QStringLiteral("🍛")},
        }},
        {QVariantMap{
            {QStringLiteral("category"), EmojiModel::Activities},
            {QStringLiteral("name"), i18nc("'Activities' is a category of emoji", "Activities")},
            {QStringLiteral("emoji"), QStringLiteral("🚁")},
        }},
        {QVariantMap{
            {QStringLiteral("category"), EmojiModel::Travel},
            {QStringLiteral("name"), i18nc("'Travel' is  a category of emoji", "Travel")},
            {QStringLiteral("emoji"), QStringLiteral("🚅")},
        }},
        {QVariantMap{
            {QStringLiteral("category"), EmojiModel::Objects},
            {QStringLiteral("name"), i18nc("'Objects' is a category of emoji", "Objects")},
            {QStringLiteral("emoji"), QStringLiteral("💡")},
        }},
        {QVariantMap{
            {QStringLiteral("category"), EmojiModel::Symbols},
            {QStringLiteral("name"), i18nc("'Symbols' is a category of emoji", "Symbols")},
            {QStringLiteral("emoji"), QStringLiteral("🔣")},
        }},
        {QVariantMap{
            {QStringLiteral("category"), EmojiModel::Flags},
            {QStringLiteral("name"), i18nc("'Flags' is a category of emoji", "Flags")},
            {QStringLiteral("emoji"), QStringLiteral("🏁")},
        }},
    };
}

QVariantList EmojiModel::categoriesWithCustom() const
{
    auto cats = categories();
    cats.removeAt(0);
    cats.insert(0,
                QVariantMap{
                    {QStringLiteral("category"), EmojiModel::History},
                    {QStringLiteral("name"), i18nc("Previously used emojis", "History")},
                    {QStringLiteral("emoji"), QStringLiteral("⌛️")},
                });
    cats.insert(1,
                QVariantMap{
                    {QStringLiteral("category"), EmojiModel::Custom},
                    {QStringLiteral("name"), i18nc("'Custom' is a category of emoji", "Custom")},
                    {QStringLiteral("emoji"), QStringLiteral("🖼️")},
                });
    ;
    return cats;
}

QVariantList EmojiModel::emojiHistory() const
{
    QVariantList list;
    const auto &lastUsed = lastUsedEmojis();
    for (const auto &historicEmoji : lastUsed) {
        for (const auto &emojiCategory : std::as_const(_emojis)) {
            for (const auto &emoji : emojiCategory) {
                if (qvariant_cast<Emoji>(emoji).shortName == historicEmoji) {
                    list.append(emoji);
                }
            }
        }
    }
    return list;
}

#include "moc_emojimodel.cpp"
