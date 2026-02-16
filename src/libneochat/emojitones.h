// SPDX-FileCopyrightText: None
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "models/emojimodel.h"

#include <QMultiHash>

/**
 * @class EmojiTones
 *
 * This class provides a _tones variable with the available emoji tones to EmojiModel.
 *
 * @sa EmojiModel
 */
class EmojiTones
{
private:
    static QMultiHash<QString, Emoji> tones();

    friend class EmojiModel;
};
