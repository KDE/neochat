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
        roleValue: DelegateType.State
        delegate: StateDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Emote
        delegate: MessageDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Message
        delegate: MessageDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Notice
        delegate: MessageDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Image
        delegate: ImageDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Sticker
        delegate: ImageDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Audio
        delegate: AudioDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Video
        delegate: VideoDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.File
        delegate: FileDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Encrypted
        delegate: EncryptedDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.ReadMarker
        delegate: ReadMarkerDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Poll
        delegate: PollDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Location
        delegate: LocationDelegate {}
    }
    DelegateChoice {
        roleValue: DelegateType.LiveLocation
        delegate: LiveLocationDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Other
        delegate: Item {}
    }
}
