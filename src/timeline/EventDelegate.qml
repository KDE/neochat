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
        delegate: StateDelegate {
            room: root.room
        }
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
        roleValue: DelegateType.Predecessor
        delegate: PredecessorDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Successor
        delegate: SuccessorDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.TimelineEnd
        delegate: TimelineEndDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Spacer
        delegate: SpacerDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Other
        delegate: NeoChatConfig.showAllEvents ? hiddenDelegate : emptyDelegate

        Component {
            id: hiddenDelegate
            HiddenDelegate {
                room: root.room
            }
        }
        Component {
            id: emptyDelegate
            Item {}
        }
    }
}
