// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import Qt.labs.qmlmodels 1.0
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

DelegateChooser {
    role: "eventType"

    DelegateChoice {
        roleValue: "state"
        delegate: StateDelegate {}
    }

    DelegateChoice {
        roleValue: "emote"
        delegate: MessageDelegate {
            isEmote: true
        }
    }

    DelegateChoice {
        roleValue: "message"
        delegate: MessageDelegate {}
    }

    DelegateChoice {
        roleValue: "notice"
        delegate: MessageDelegate {}
    }

    DelegateChoice {
        roleValue: "image"
        delegate: ImageDelegate {}
    }

    DelegateChoice {
        roleValue: "sticker"
        delegate: ImageDelegate {
            cardBackground: false
        }
    }

    DelegateChoice {
        roleValue: "audio"
        delegate: AudioDelegate {}
    }

    DelegateChoice {
        roleValue: "video"
        delegate: VideoDelegate {}
    }

    DelegateChoice {
        roleValue: "file"
        delegate: FileDelegate {}
    }

    DelegateChoice {
        roleValue: "encrypted"
        delegate: EncryptedDelegate {}
    }

    DelegateChoice {
        roleValue: "readMarker"
        delegate: ReadMarkerDelegate {}
    }

    DelegateChoice {
        roleValue: "other"
        delegate: Item {}
    }
}
