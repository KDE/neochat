// SPDX-FileCopyrightText: 2025 Ritchie Frodomar <alkalinethunder@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma Singleton

import QtQuick
import QtTextToSpeech

QtObject {
    id: root

    readonly property TextToSpeech tts: TextToSpeech {
        id: tts
    }

    function warmUp() {
        // TODO: This method is called on startup to avoid a UI freeze the first time you read a message aloud, but there's nothing for it to do.
        // This would be a good place to check if TTS can actually be used.
    }

    function say(text: String) {
        tts.say(text)
    }
}