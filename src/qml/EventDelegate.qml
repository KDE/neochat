// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Layouts

import Qt.labs.qmlmodels

import org.kde.neochat

DelegateChooser {
    id: root

    role: "delegateType"
    property var room
    required property NeoChatConnection connection

    /**
     * @brief Whether the delegate is part of a thread timeline.
     */
    property bool isThread: false

    DelegateChoice {
        roleValue: DelegateType.State
        delegate: StateDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Emote
        delegate: TextDelegate {
            connection: root.connection
            isThread: root.isThread
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Message
        delegate: TextDelegate {
            connection: root.connection
            isThread: root.isThread
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Notice
        delegate: TextDelegate {
            connection: root.connection
            isThread: root.isThread
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Image
        delegate: ImageDelegate {
            connection: root.connection
            isThread: root.isThread
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Sticker
        delegate: ImageDelegate {
            isThread: root.isThread
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Audio
        delegate: AudioDelegate {
            connection: root.connection
            isThread: root.isThread
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Video
        delegate: VideoDelegate {
            connection: root.connection
            isThread: root.isThread
        }
    }

    DelegateChoice {
        roleValue: DelegateType.File
        delegate: FileDelegate {
            connection: root.connection
            isThread: root.isThread
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Encrypted
        delegate: EncryptedDelegate {
            connection: root.connection
            isThread: root.isThread
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
            isThread: root.isThread
        }
    }
    DelegateChoice {
        roleValue: DelegateType.LiveLocation
        delegate: LiveLocationDelegate {
            room: root.room
            connection: root.connection
            isThread: root.isThread
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Other
        delegate: Item {}
    }
}
