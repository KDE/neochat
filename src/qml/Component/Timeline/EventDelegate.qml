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
        roleValue: MessageEventModel.State
        delegate: StateDelegate {}
    }

    DelegateChoice {
        roleValue: MessageEventModel.Emote
        delegate: MessageDelegate {
            isEmote: true
        }
    }

    DelegateChoice {
        roleValue: MessageEventModel.Message
        delegate: MessageDelegate {}
    }

    DelegateChoice {
        roleValue: MessageEventModel.Notice
        delegate: MessageDelegate {}
    }

    DelegateChoice {
        roleValue: MessageEventModel.Image
        delegate: ImageDelegate {}
    }

    DelegateChoice {
        roleValue: MessageEventModel.Sticker
        delegate: ImageDelegate {}
    }

    DelegateChoice {
        roleValue: MessageEventModel.Audio
        delegate: AudioDelegate {}
    }

    DelegateChoice {
        roleValue: MessageEventModel.Video
        delegate: VideoDelegate {}
    }

    DelegateChoice {
        roleValue: MessageEventModel.File
        delegate: FileDelegate {}
    }

    DelegateChoice {
        roleValue: MessageEventModel.Encrypted
        delegate: EncryptedDelegate {}
    }

    DelegateChoice {
        roleValue: MessageEventModel.ReadMarker
        delegate: ReadMarkerDelegate {}
    }

    DelegateChoice {
        roleValue: MessageEventModel.Other
        delegate: Item {}
    }
}
