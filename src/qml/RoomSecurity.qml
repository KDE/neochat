// SPDX-FileCopyrightText: 2019-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    property NeoChatRoom room
    property string needUpgradeRoom: i18n("You need to upgrade this room to a newer version to enable this setting.")

    title: i18n("Security")

    FormCard.FormHeader {
        title: i18nc("@option:check", "Encryption")
    }
    FormCard.FormCard {
        FormCard.FormSwitchDelegate {
            id: enableEncryptionSwitch
            text: i18n("Enable encryption")
            description: i18nc("option:check", "Once enabled, encryption cannot be disabled.")
            enabled: room.canEncryptRoom
            checked: room.usesEncryption
            onToggled: if (checked) {
                let dialog = confirmEncryptionDialog.createObject(applicationWindow().overlay, {room: room});
                dialog.open();
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@option:check", "Access")
    }
    FormCard.FormCard {
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Private (invite only)")
            description: i18n("Only invited people can join.")
            checked: room.joinRule === "invite"
            enabled: room.canSendState("m.room.join_rules")
            onCheckedChanged: if (checked) {
                room.joinRule = "invite";
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Space members")
            description: i18n("Anyone in a space can find and join.") +
                            (!["8", "9", "10"].includes(room.version) ? `\n${needUpgradeRoom}` : "")
            checked: room.joinRule === "restricted"
            enabled: room.canSendState("m.room.join_rules") && ["8", "9", "10"].includes(room.version) && false
            onCheckedChanged: if (checked) {
                room.joinRule = "restricted";
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Knock")
            description: i18n("People not in the room need to request an invite to join the room.") +
                            (!["7", "8", "9", "10"].includes(room.version) ? `\n${needUpgradeRoom}` : "")
            checked: room.joinRule === "knock"
            // https://spec.matrix.org/v1.4/rooms/#feature-matrix
            enabled: room.canSendState("m.room.join_rules") && ["7", "8", "9", "10"].includes(room.version)
            onCheckedChanged: if (checked) {
                room.joinRule = "knock";
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Public")
            description: i18nc("@option:check", "Anyone can find and join.")
            checked: room.joinRule === "public"
            enabled: room.canSendState("m.room.join_rules")
            onCheckedChanged: if (checked) {
                room.joinRule = "public";
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@option:check", "Message history visibility")
    }
    FormCard.FormCard {
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Anyone")
            description: i18nc("@option:check", "Anyone, regardless of whether they have joined, can view history.")
            checked: room.historyVisibility === "world_readable"
            enabled: room.canSendState("m.room.history_visibility")
            onCheckedChanged: if (checked) {
                room.historyVisibility = "world_readable"
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Members only")
            description: i18nc("@option:check", "All members can view the entire message history, even before they joined.")
            checked: room.historyVisibility === "shared"
            enabled: room.canSendState("m.room.history_visibility")
            onCheckedChanged: if (checked) {
                room.historyVisibility = "shared"
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Members only (since invite)")
            description: i18nc("@option:check", "New members can view the message history from the point they were invited to the room.")
            checked: room.historyVisibility === "invited"
            enabled: room.canSendState("m.room.history_visibility")
            onCheckedChanged: if (checked) {
                room.historyVisibility = "invited"
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Members only (since joining)")
            description: i18nc("@option:check", "New members can view the message history from the point they joined the room.")
            checked: room.historyVisibility === "joined"
            enabled: room.canSendState("m.room.history_visibility")
            onCheckedChanged: if (checked) {
                room.historyVisibility = "joined"
            }
        }
    }

    property Component confirmEncryptionDialog: Component {
        id: confirmEncryptionDialog

        ConfirmEncryptionDialog {
            onClosed: {
                // At the point this is executed, the state in the room is not yet changed.
                // The value will be updated when room.onEncryption() emitted.
                // This is in case if user simply closed the dialog.
                enableEncryptionSwitch.checked = false
            }
        }
    }

    property Connections connections: Connections {
        target: room
        onEncryption: {
            enableEncryptionSwitch.checked = room.usesEncryption
        }
    }
}

