// SPDX-FileCopyrightText: None
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "emojitones.h"
#include "models/emojimodel.h"

QMultiHash<QString, QVariant> EmojiTones::_tones = {
#include "emojitones_data.h"
};
