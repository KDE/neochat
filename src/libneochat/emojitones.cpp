// SPDX-FileCopyrightText: None
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "emojitones.h"

struct {
    const char8_t *name;
    const char8_t *escaped_sequence;
    const char8_t *shortcode;
    const char8_t *description;
} constexpr const tones_data[] = {
#include "emojitones_data.h"
};

using namespace Qt::StringLiterals;

QMultiHash<QString, Emoji> EmojiTones::tones()
{
    static QMultiHash<QString, Emoji> _tones;
    if (_tones.isEmpty()) {
        for (const auto &tone : tones_data) {
            _tones.insert(QString::fromUtf8(tone.name),
                          Emoji(QString::fromUtf8(tone.escaped_sequence), QString::fromUtf8(tone.shortcode), QString::fromUtf8(tone.description)));
        }
    }
    return _tones;
}
