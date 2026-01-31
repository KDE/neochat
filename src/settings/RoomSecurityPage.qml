// SPDX-FileCopyrightText: 2019-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

import io.github.quotient_im.libquotient

FormCard.FormCardPage {
    id: root

    property NeoChatRoom room
    property string needUpgradeRoom: i18nc("@info", "You need to upgrade this room to a newer version to enable this setting.")

    title: i18nc("@title", "Security")

    FormCard.FormHeader {
        title: i18nc("@option:check", "Encryption")
    }
    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            visible: root.room.usesEncryption

            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing
                Kirigami.Icon {
                    source: "lock"
                }
                QQC2.Label {
                    text: i18nc("@info", "This room uses encryption.")
                    wrapMode: Text.WordWrap

                    Layout.fillWidth: true
                }
            }
        }
        FormCard.FormButtonDelegate {
            id: enableEncryptionSwitch

            icon.name: "lock-symbolic"
            text: i18nc("@action:button Enable encryption in this room", "Enable Encryption…")
            description: i18nc("@info:description", "Once enabled, encryption cannot be disabled.")
            enabled: root.room.canEncryptRoom
            visible: !root.room.usesEncryption

            onClicked: (confirmEncryptionDialog.createObject(QQC2.Overlay.overlay, {
                room: root.room
            }) as ConfirmEncryptionDialog).open()
        }
    }

    FormCard.FormHeader {
        title: i18nc("@option:check", "Access")
    }
    FormCard.FormCard {
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Private (invite only)")
            description: i18nc("@info", "Only invited people can join.")
            checked: root.room.joinRule === JoinRule.Invite
            enabled: root.room.canSendState("m.room.join_rules")
            onCheckedChanged: if (checked && root.room.joinRule != JoinRule.Invite) {
                root.room.joinRule = JoinRule.Invite;
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Space members")
            description: i18nc("@info", "Anyone in the selected spaces can find and join.") + (!["8", "9", "10", "11", "12"].includes(root.room.version) ? `\n${root.needUpgradeRoom}` : "")
            checked: root.room.joinRule === JoinRule.Restricted
            enabled: root.room.canSendState("m.room.join_rules") && ["8", "9", "10", "11", "12"].includes(root.room.version)
            onCheckedChanged: if (checked && root.room.joinRule != JoinRule.Restricted) {
                (selectSpacesDialog.createObject(QQC2.Overlay.overlay) as SelectSpacesDialog).open();
            }

            contentItem.children: QQC2.Button {
                visible: root.room.joinRule === JoinRule.Restricted
                text: i18nc("@action:button", "Select spaces")
                icon.name: "list-add"

                onClicked: (selectSpacesDialog.createObject(QQC2.Overlay.overlay) as SelectSpacesDialog).open()

                QQC2.ToolTip.text: text
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
            }

            Component {
                id: selectSpacesDialog
                SelectSpacesDialog {
                    room: root.room
                }
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Knock")
            description: i18nc("@info", "People not in the room need to request an invite to join the room.") + (!["7", "8", "9", "10", "11", "12"].includes(root.room.version) ? `\n${root.needUpgradeRoom}` : "")
            checked: root.room.joinRule === JoinRule.Knock
            // https://spec.matrix.org/v1.4/rooms/#feature-matrix
            enabled: root.room.canSendState("m.room.join_rules") && ["7", "8", "9", "10", "11", "12"].includes(root.room.version)
            onCheckedChanged: if (checked && root.room.joinRule != JoinRule.Knock) {
                root.room.joinRule = JoinRule.Knock;
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Public")
            description: i18nc("@option:check", "Anyone can find and join.")
            checked: root.room.joinRule === JoinRule.Public
            enabled: root.room.canSendState("m.room.join_rules")
            onCheckedChanged: if (checked && root.room.joinRule != JoinRule.Public) {
                root.room.joinRule = JoinRule.Public;
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
            checked: root.room.historyVisibility === "world_readable"
            enabled: root.room.canSendState("m.room.history_visibility")
            onCheckedChanged: if (checked) {
                root.room.historyVisibility = "world_readable";
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Members only")
            description: i18nc("@option:check", "All members can view the entire message history, even before they joined.")
            checked: root.room.historyVisibility === "shared"
            enabled: root.room.canSendState("m.room.history_visibility")
            onCheckedChanged: if (checked) {
                root.room.historyVisibility = "shared";
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Members only (since invite)")
            description: i18nc("@option:check", "New members can view the message history from the point they were invited to the room.")
            checked: root.room.historyVisibility === "invited"
            enabled: root.room.canSendState("m.room.history_visibility")
            onCheckedChanged: if (checked) {
                root.room.historyVisibility = "invited";
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("@option:check", "Members only (since joining)")
            description: i18nc("@option:check", "New members can view the message history from the point they joined the room.")
            checked: root.room.historyVisibility === "joined"
            enabled: root.room.canSendState("m.room.history_visibility")
            onCheckedChanged: if (checked) {
                root.room.historyVisibility = "joined";
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
                enableEncryptionSwitch.checked = false;
            }
        }
    }

    property Connections connections: Connections {
        target: root.room
        function onEncryption() {
            enableEncryptionSwitch.checked = root.room.usesEncryption;
        }
    }
}
