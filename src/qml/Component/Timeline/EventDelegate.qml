// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Qt.labs.qmlmodels 1.0

import org.kde.neochat 1.0

DelegateChooser {
    id: root
    role: "delegateType"
    property var room

    DelegateChoice {
        roleValue: MessageEventModel.State
        delegate: StateDelegate {}
    }

    DelegateChoice {
        roleValue: MessageEventModel.Emote
        delegate: MessageDelegate {}
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
        roleValue: MessageEventModel.Poll
        delegate: PollDelegate {}
    }

    DelegateChoice {
        roleValue: MessageEventModel.Location
        delegate: LocationDelegate {}
    }
    DelegateChoice {
        roleValue: MessageEventModel.LiveLocation
        delegate: LiveLocationDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: MessageEventModel.Other
        delegate: Item {}
    }
}
