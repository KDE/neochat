// SPDX-FileCopyrightText: None
// SPDX-License-Identifier: LGPL-2.0-or-later
// This file is auto-generated. All changes will be lost. See tools/README.md
// clang-format off

#include <QString>
#include <QHash>
#include "../../emojimap.h"

class astEmojiMap: public EmojiMap {

public:
    QHash<EmojiModel::Category, QVector<Emoji>> langEmojiMap()
    {
        QHash<EmojiModel::Category, QVector<Emoji>> _emojis;
        _emojis[EmojiModel::Smileys].append(Emoji{QString::fromUtf8("\U0001F923"), QStringLiteral("rodando pel suelu de risa")});
        _emojis[EmojiModel::Smileys].append(Emoji{QString::fromUtf8("\U0001F602"), QStringLiteral("cara llorando d’allegría")});
        _emojis[EmojiModel::Smileys].append(Emoji{QString::fromUtf8("\U0001F618"), QStringLiteral("cara tirando un besu")});
        _emojis[EmojiModel::Smileys].append(Emoji{QString::fromUtf8("\U0001F617"), QStringLiteral("cara besando")});
        _emojis[EmojiModel::Smileys].append(Emoji{QString::fromUtf8("\U0000263A"), QStringLiteral("cara sorriendo")});
        _emojis[EmojiModel::People].append(Emoji{QString::fromUtf8("\U0001F933"), QStringLiteral("selfie")});
        _emojis[EmojiModel::People].append(Emoji{QString::fromUtf8("\U0001F445"), QStringLiteral("llingua")});
        _emojis[EmojiModel::People].append(Emoji{QString::fromUtf8("\U0001F444"), QStringLiteral("boca")});
        _emojis[EmojiModel::People].append(Emoji{QString::fromUtf8("\U0001F48F"), QStringLiteral("besu")});
        _emojis[EmojiModel::People].append(Emoji{QString::fromUtf8("\U0001F46A"), QStringLiteral("familia")});
        _emojis[EmojiModel::Objects].append(Emoji{QString::fromUtf8("\U0001F48D"), QStringLiteral("aniellu")});
        _emojis[EmojiModel::Objects].append(Emoji{QString::fromUtf8("\U0001F48E"), QStringLiteral("piedra preciosa")});
};
