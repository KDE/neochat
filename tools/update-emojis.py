#!/usr/bin/python3
# SPDX-FileCopyrightText: 2022-2024 Tobias Fella <tobias.fella@kde.org>
# SPDX-FileCopyrightText: 2022 Gary Wang <wzc782970009@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

import requests
import json
import os

# This is nicer data, but in a useless order
data = requests.get('https://raw.githubusercontent.com/joypixels/emoji-toolkit/master/emoji_strategy.json').json()

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

categories = dict()
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
tones = []
for e in data:
    codepoint = data[e]["unicode_output"].replace("-", "")
    print(codepoint)
    codepoint = "\\x" + "\\x".join([codepoint[i:i+2] for i in range(0, len(codepoint), 2)])
    print(codepoint)
    codepoint = codepoint.encode().decode('unicode-escape').encode('latin1').decode('utf-8')
    print(codepoint)

    file.write(f"{codepoint};{data[e]["name"]};{data[e]["shortname"]};{data[e]["category"]}\n")

file.close()
tones_file.close()
