#!/usr/bin/python3
# SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
# SPDX-FileCopyrightText: 2022 Gary Wang <wzc782970009@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

import requests
import json
import os

# This is nicer data, but in a useless order
data = requests.get('https://raw.githubusercontent.com/bonusly/gemojione/master/config/index.json').json()

# This is worse data, but in a better order. So we're using the order from this data, with the data from the other source
unicode_emoji_data = requests.get('https://unicode.org/Public/emoji/14.0/emoji-test.txt')

file = open(os.path.dirname(os.path.abspath(__file__)) + "/../src/data/emojis.data", "w+")
# REUSE-IgnoreStart
file.write("// SPDX-FileCopyrightText: None\n")
file.write("// SPDX-License-Identifier: LGPL-2.0-or-later\n")
# REUSE-IgnoreEnd
file.write("// This file is auto-generated. All changes will be lost. See tools/update-emojis.py\n")

tones_file = open(os.path.dirname(os.path.abspath(__file__)) + "/../src/data/emojitones_data.h", "w+")
# REUSE-IgnoreStart
tones_file.write("// SPDX-FileCopyrightText: None\n")
tones_file.write("// SPDX-License-Identifier: LGPL-2.0-or-later\n")
# REUSE-IgnoreEnd
tones_file.write("// This file is auto-generated. All changes will be lost. See tools/update-emojis.py\n")

def escape_sequence(unicode_str: str, codepoint_spliter: str) -> str:
    codepoints = unicode_str.split(codepoint_spliter)
    escape_sequence = ""
    for codepoint in codepoints:
        escape_sequence += "\\U" + codepoint.rjust(8, "0")
    return escape_sequence


categories = {}

categoryies = dict()
categories["people"] = ("People & Emotion", "🙋‍♂️")
categories["food"] = ("Food & Drink", "🍛")
categories["nature"] = ("Animals & Nature", "🌲")
categories["travel"] = ("Travel & Places", "🚅")
categories["activity"] = ("Activities", "🚁")
categories["objects"] = ("Objects", "💡")
categories["flags"] = ("Flags", "🏁")
categories["symbols"] = ("Symbols", "🔣")

for category in categories:
    name, icon = categories[category]
    file.write(f"##{icon};{name};{category}\n")

emojis = dict()
for e in data:
    emoji = e
    unicode = data[emoji]["moji"]
    name = data[emoji]["name"]
    name = name.replace("tone 1", "with light skin tone")
    name = name.replace("tone 2", "with medium-light skin tone")
    name = name.replace("tone 3", "with medium skin tone")
    name = name.replace("tone 4", "with medium-dark skin tone")
    name = name.replace("tone 5", "with medium-dark skin tone")
    name = name.capitalize()
    emoji = emoji.replace("_tone1", "_light_skin_tone")
    emoji = emoji.replace("_tone2", "_medium_light_skin_tone")
    emoji = emoji.replace("_tone3", "_medium_skin_tone")
    emoji = emoji.replace("_tone4", "_medium_dark_skin_tone")
    emoji = emoji.replace("_tone5", "_dark_skin_tone")

    category = data[e]["category"]
    if category == "extras":
        category = "flags"
    if category == "places":
        category = "activity"
    if category == "modifier" or category == "regional":
        continue
    emojis[escape_sequence(data[e]['unicode'], "-")] = (unicode, name, emoji, category)

for line in unicode_emoji_data.text.split("\n"):
    if line.startswith("#") or line == "":
        continue
    parts = line.split(";")
    first = parts[0].strip()
    escaped_sequence = escape_sequence(first, " ")
    if escaped_sequence in emojis:
        icon, name, emoji, category = emojis.pop(escaped_sequence)
        file.write(f"{icon};{name};{emoji};{category}\n")

for shortcode, (icon, name, emoji, category) in emojis.items():
    file.write(f"{icon};{name};{emoji};{category}\n")

file.close()
tones_file.close()
