// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@¢arlschwan.eu>

#include "emojimodel.h"

class EmojiMap
{
    QHash<EmojiModel::Category, QVector<Emoji>> langEmojiMap();
};
