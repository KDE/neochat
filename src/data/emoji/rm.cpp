// SPDX-FileCopyrightText: None
// SPDX-License-Identifier: LGPL-2.0-or-later
// This file is auto-generated. All changes will be lost. See tools/README.md
// clang-format off

#include <QString>
#include <QHash>
#include "../../emojimap.h"

class rmEmojiMap: public EmojiMap {

public:
    QHash<EmojiModel::Category, QVector<Emoji>> langEmojiMap()
    {
        QHash<EmojiModel::Category, QVector<Emoji>> _emojis;
        _emojis[EmojiModel::People].append(Emoji{QString::fromUtf8("\U0001F935\U0000200D\U00002642"), QStringLiteral("um cun smoking")});
        _emojis[EmojiModel::People].append(Emoji{QString::fromUtf8("\U0001F935\U0000200D\U00002640"), QStringLiteral("dunna cun smoking")});
        _emojis[EmojiModel::People].append(Emoji{QString::fromUtf8("\U0001F470\U0000200D\U00002642"), QStringLiteral("um cun vel")});
        _emojis[EmojiModel::People].append(Emoji{QString::fromUtf8("\U0001F470\U0000200D\U00002640"), QStringLiteral("dunna cun vel")});
        _emojis[EmojiModel::Component].append(Emoji{QString::fromUtf8("\U0001F3FB"), QStringLiteral("pel clera")});
        _emojis[EmojiModel::Component].append(Emoji{QString::fromUtf8("\U0001F3FC"), QStringLiteral("pel mez clera")});
        _emojis[EmojiModel::Component].append(Emoji{QString::fromUtf8("\U0001F3FD"), QStringLiteral("pel media")});
        _emojis[EmojiModel::Component].append(Emoji{QString::fromUtf8("\U0001F3FE"), QStringLiteral("pel mez stgira")});
        _emojis[EmojiModel::Component].append(Emoji{QString::fromUtf8("\U0001F3FF"), QStringLiteral("pel stgira")});
        _emojis[EmojiModel::People].append(Emoji{QString::fromUtf8("\U0001F935"), QStringLiteral("persuna cun smoking")});
        _emojis[EmojiModel::People].append(Emoji{QString::fromUtf8("\U0001F470"), QStringLiteral("persuna cun vel")});
        _emojis[EmojiModel::Component].append(Emoji{QString::fromUtf8("\U0001F9B0"), QStringLiteral("chavels cotschens")});
        _emojis[EmojiModel::Component].append(Emoji{QString::fromUtf8("\U0001F9B1"), QStringLiteral("chavels ritschads")});
        _emojis[EmojiModel::Nature].append(Emoji{QString::fromUtf8("\U0001F435"), QStringLiteral("fatscha da schimgia")});
        _emojis[EmojiModel::Nature].append(Emoji{QString::fromUtf8("\U0001F412"), QStringLiteral("schimgia")});
        _emojis[EmojiModel::Nature].append(Emoji{QString::fromUtf8("\U0001F98D"), QStringLiteral("gorilla")});
        _emojis[EmojiModel::Nature].append(Emoji{QString::fromUtf8("\U0001F9A7"), QStringLiteral("orangutan")});
        _emojis[EmojiModel::Nature].append(Emoji{QString::fromUtf8("\U0001F436"), QStringLiteral("fatscha da chaun")});
        _emojis[EmojiModel::Nature].append(Emoji{QString::fromUtf8("\U0001F415"), QStringLiteral("chaun")});
        _emojis[EmojiModel::Nature].append(Emoji{QString::fromUtf8("\U0001F429"), QStringLiteral("pudel")});
};
