#!/bin/python

# SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
# SPDX-FileCopyrightText: 2022 Gary Wang <wzc782970009@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

import requests
import re


def escape_sequence(unicode_str: str, codepoint_spliter: str) -> str:
    codepoints = unicode_str.split(codepoint_spliter)
    escape_sequence = ""
    for codepoint in codepoints:
        escape_sequence += "\\U" + codepoint.rjust(8, "0")
    return escape_sequence


# GitLab uses the emoji shortnames from Gemojione
# See also: https://docs.gitlab.com/ee/development/fe_guide/emojis.html
gemojione = requests.get('https://raw.githubusercontent.com/bonusly/gemojione/master/config/index.json')
emoji_unicode_shortname_map = {}
gemojione_json = gemojione.json()
for (shortcode, props) in gemojione_json.items():
    escaped_sequence = escape_sequence(props['unicode'], "-")
    emoji_unicode_shortname_map[escaped_sequence] = shortcode


response = requests.get('https://unicode.org/Public/emoji/14.0/emoji-test.txt')
group = ""
file = open("../src/emojis.h", "w")
# REUSE-IgnoreStart
file.write("// SPDX-FileCopyrightText: None\n")
file.write("// SPDX-License-Identifier: LGPL-2.0-or-later\n")
# REUSE-IgnoreEnd
file.write("// This file is auto-generated. All changes will be lost. See tools/update-emojis.py\n")
file.write("// clang-format off\n")

tones_file = open("../src/emojitones_data.h", "w")
# REUSE-IgnoreStart
tones_file.write("// SPDX-FileCopyrightText: None\n")
tones_file.write("// SPDX-License-Identifier: LGPL-2.0-or-later\n")
# REUSE-IgnoreEnd
tones_file.write("// This file is auto-generated. All changes will be lost. See tools/update-emojis.py\n")
tones_file.write("// clang-format off\n")

for line in response.text.split("\n"):
    if line.startswith("# group"):
        raw_group = line.split(": ")[1]
        if raw_group == "Activities":
            group = "Activities"
        elif raw_group == "Animals & Nature":
            group = "Nature"
        elif raw_group == "Component":
            group = "Component"
        elif raw_group == "Flags":
            group = "Flags"
        elif raw_group == "Food & Drink":
            group = "Food"
        elif raw_group == "Objects":
            group = "Objects"
        elif raw_group == "People & Body":
            group = "People"
        elif raw_group == "Smileys & Emotion":
            group = "Smileys"
        elif raw_group == "Symbols":
            group = "Symbols"
        elif raw_group == "Travel & Places":
            group = "Travel"
        else:
            print("Unknown group:" + group)
            group = ""
    elif line.startswith("#") or line == "":
        pass
    else:
        parts = line.split(";")
        first = parts[0].strip()
        escaped_sequence = escape_sequence(first, " ")

        x = re.search(".*E[0-9]+.[0-9] ", parts[1])
        description = parts[1].removeprefix(x.group())
        shortcode = description
        if "flag:" in description:
            description = "Flag of " + description.split(": ")[1]

        if "unqualified" in line or "minimally-qualified" in line:
            continue

        is_skin_tone = "skin tone" in description

        if escaped_sequence in emoji_unicode_shortname_map:
            shortcode = emoji_unicode_shortname_map[escaped_sequence]

        emoji_args = 'QString::fromUtf8("{0}"), QStringLiteral("{1}"), QStringLiteral("{2}")'.format(escaped_sequence, shortcode, description)
        emoji_qvariant = 'QVariant::fromValue(Emoji{' + emoji_args + '})'

        if is_skin_tone:
            tones_file.write("{\"" + description.split(":")[0] + "\", " + emoji_qvariant + "},\n")
            continue
        file.write("_emojis[" + group + "].append(" + emoji_qvariant + ");\n")
file.close()
tones_file.close()
