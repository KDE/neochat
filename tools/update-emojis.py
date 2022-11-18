#!/bin/python

# SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
# SPDX-License-Identifier: BSD-2-Clause

import requests
import re

response = requests.get('https://unicode.org/Public/emoji/14.0/emoji-test.txt')
group = ""
file = open("../src/emojis.h", "w")
# AAAAARGH reusetool
file.write("// SPDX")
file.write("-FileCopyrightText: None\n")
file.write("// SPDX")
file.write("-License-Identifier: LGPL-2.0-or-later\n")
file.write("// This file is auto-generated. All changes will be lost. See tools/update-emojis.py\n")
file.write("// clang-format off\n")

tones_file = open("../src/emojitones.h", "w")
tones_file.write("// SPDX")
tones_file.write("-FileCopyrightText: None\n")
tones_file.write("// SPDX")
tones_file.write("-License-Identifier: LGPL-2.0-or-later\n")
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
        codepoints = first.split(" ")

        x = re.search(".*E[0-9]+.[0-9] ", parts[1])
        description = parts[1].removeprefix(x.group())
        if "flag:" in description:
            description = "Flag of " + description.split(": ")[1]

        escape_sequence = ""
        for codepoint in codepoints:
            escape_sequence += "\\U" + codepoint.rjust(8, "0")

        if "unqualified" in line or "minimally-qualified" in line:
            continue
        if "skin tone" in description:
            tones_file.write("{\"" + description.split(":")[0] + "\", QVariant::fromValue(Emoji{QString::fromUtf8(\"" + escape_sequence + "\"), QStringLiteral(\"" + description + "\")})},\n")
            continue
        file.write("_emojis[" + group + "].append(QVariant::fromValue(Emoji{QString::fromUtf8(\"" + escape_sequence + "\"), QStringLiteral(\"" + description + "\")}));\n")
file.close()
tones_file.close()
