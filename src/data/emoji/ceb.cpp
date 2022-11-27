// SPDX-FileCopyrightText: None
// SPDX-License-Identifier: LGPL-2.0-or-later
// This file is auto-generated. All changes will be lost. See tools/README.md
// clang-format off

#include <QString>
#include <QHash>
#include "../../emojimap.h"

class cebEmojiMap: public EmojiMap {

public:
    QHash<EmojiModel::Category, QVector<Emoji>> langEmojiMap()
    {
        QHash<EmojiModel::Category, QVector<Emoji>> _emojis;
        _emojis[EmojiModel::Flags].append(Emoji{QString::fromUtf8("\U0001F3F3\U0000200D\U000026A7"), QStringLiteral("flag sa transgender")});
};
