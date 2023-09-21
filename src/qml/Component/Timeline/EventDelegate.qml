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
    required property NeoChatConnection connection

    DelegateChoice {
        roleValue: DelegateType.State
        delegate: StateDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Emote
        delegate: TextDelegate {
            connection: root.connection
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Message
        delegate: TextDelegate {
            connection: root.connection
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Notice
        delegate: TextDelegate {
            connection: root.connection
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Image
        delegate: ImageDelegate {
            connection: root.connection
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Sticker
        delegate: ImageDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Audio
        delegate: AudioDelegate {
            connection: root.connection
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Video
        delegate: VideoDelegate {
            connection: root.connection
        }
    }

    DelegateChoice {
        roleValue: DelegateType.File
        delegate: FileDelegate {
            connection: root.connection
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Encrypted
        delegate: EncryptedDelegate {
            connection: root.connection
        }
    }

    DelegateChoice {
        roleValue: DelegateType.ReadMarker
        delegate: ReadMarkerDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Poll
        delegate: PollDelegate {
            connection: root.connection
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Location
        delegate: LocationDelegate {
            connection: root.connection
        }
    }
    DelegateChoice {
        roleValue: DelegateType.LiveLocation
        delegate: LiveLocationDelegate {
            room: root.room
            connection: root.connection
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Other
        delegate: Item {}
    }
}
