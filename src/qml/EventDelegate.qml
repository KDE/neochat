// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Layouts

import Qt.labs.qmlmodels

import org.kde.neochat

DelegateChooser {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    role: "delegateType"

    DelegateChoice {
        roleValue: DelegateType.State
        delegate: StateDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Emote
        delegate: TextDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Message
        delegate: TextDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Notice
        delegate: TextDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Image
        delegate: ImageDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Sticker
        delegate: ImageDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Audio
        delegate: AudioDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Video
        delegate: VideoDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.File
        delegate: FileDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Encrypted
        delegate: EncryptedDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.ReadMarker
        delegate: ReadMarkerDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Poll
        delegate: PollDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Location
        delegate: LocationDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.LiveLocation
        delegate: LiveLocationDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Loading
        delegate: LoadingDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.TimelineEnd
        delegate: TimelineEndDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Other
        delegate: Item {}
    }
}
