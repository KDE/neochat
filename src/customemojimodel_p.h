// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "customemojimodel.h"

struct CustomEmoji
{
    QString name; // with :semicolons:
    QString url; // mxc://
    QRegularExpression regexp;
};

struct CustomEmojiModel::Private
{
    Connection* conn = nullptr;
    QList<CustomEmoji> emojies;
};
