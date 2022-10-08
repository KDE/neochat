// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "customemojimodel.h"
#include <QRegularExpression>
#include <connection.h>

struct CustomEmoji {
    QString name; // with :semicolons:
    QString url; // mxc://
    QRegularExpression regexp;
};

struct CustomEmojiModel::Private {
    Quotient::Connection *conn = nullptr;
    QList<CustomEmoji> emojies;
};
