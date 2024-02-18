// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
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
        roleValue: DelegateType.Message
        delegate: MessageDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.ReadMarker
        delegate: ReadMarkerDelegate {}
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
