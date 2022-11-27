// SPDX-FileCopyrightText: None
// SPDX-License-Identifier: LGPL-2.0-or-later
// This file is auto-generated. All changes will be lost. See tools/README.md
// clang-format off

#include <QString>
#include <QHash>
#include "../../emojimap.h"

class ckbEmojiMap: public EmojiMap {

public:
    QHash<EmojiModel::Category, QVector<Emoji>> langEmojiMap()
    {
        QHash<EmojiModel::Category, QVector<Emoji>> _emojis;
        _emojis[EmojiModel::Objects].append(Emoji{QString::fromUtf8("\U0001F52B"), QStringLiteral("دەمانچەی ئاوی")});
};
