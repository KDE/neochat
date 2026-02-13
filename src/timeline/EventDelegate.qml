// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-3.0-only

pragma ComponentBehavior: Bound

import QtQuick

import Qt.labs.qmlmodels

import org.kde.neochat

DelegateChooser {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief Whether to show selection controls for message delegate.
     */
    property bool showSelectionControl: false

    role: "delegateType"

    DelegateChoice {
        roleValue: DelegateType.State
        delegate: StateDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Message
        delegate: MessageDelegate {
            showSelectionControl: root.showSelectionControl
        }
    }

    DelegateChoice {
        roleValue: DelegateType.ReadMarker
        delegate: ReadMarkerDelegate {
            room: root.room
        }
    }

    DelegateChoice {
        roleValue: DelegateType.Loading
        delegate: LoadingDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Predecessor
        delegate: PredecessorDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Successor
        delegate: SuccessorDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.TimelineEnd
        delegate: TimelineEndDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Spacer
        delegate: SpacerDelegate {}
    }

    DelegateChoice {
        roleValue: DelegateType.Other
        delegate: NeoChatConfig.showAllEvents ? hiddenDelegate : emptyDelegate

        property Component hiddenDelegate : Component {
            HiddenDelegate {}
        }
        property Component emptyDelegate : Component {
            Item {}
        }
    }
}
