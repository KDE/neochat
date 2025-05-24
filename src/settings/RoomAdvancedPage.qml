// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    property NeoChatRoom room

    title: i18nc("@window:title", "Advanced")

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.gridUnit

        FormCard.FormTextDelegate {
            id: roomIdDelegate
            text: i18nc("@info:label", "Room ID")
            description: room.id

            contentItem.children: QQC2.Button {
                visible: roomIdDelegate.hovered
                text: i18nc("@action:button", "Copy room ID to clipboard")
                icon.name: "edit-copy"
                display: QQC2.AbstractButton.IconOnly

                onClicked: {
                    Clipboard.saveText(room.id);
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
            }
        }
        FormCard.FormTextDelegate {
            text: i18nc("@info:label", "Room Version")
            description: room.version

            contentItem.children: QQC2.Button {
                visible: room.canSwitchVersions()
                enabled: room.version < room.maxRoomVersion
                text: i18nc("@action:button", "Upgrade Room")
                icon.name: "arrow-up-double"

                onClicked: {
                    if (room.canSwitchVersions()) {
                        roomUpgradeDialog.currentRoomVersion = room.version;
                        roomUpgradeDialog.open();
                    }
                }

                QQC2.ToolTip {
                    text: text
                    delay: Kirigami.Units.toolTipDelay
                }
            }
        }
    }

    property Kirigami.Dialog roomUpgradeDialog: Kirigami.Dialog {
        id: roomUpgradeDialog

        property var currentRoomVersion

        width: Kirigami.Units.gridUnit * 16

        title: i18nc("@title", "Upgrade the Room")
        ColumnLayout {
            FormCard.FormSpinBoxDelegate {
                id: spinBox
                label: i18nc("@label:spinbox", "Select new version")
                from: room.version
                to: room.maxRoomVersion
                value: room.version
            }
        }
        customFooterActions: [
            Kirigami.Action {
                text: i18nc("@action:button", "Confirm")
                icon.name: "dialog-ok"
                onTriggered: {
                    room.switchVersion(spinBox.value);
                    roomUpgradeDialog.close();
                }
            }
        ]
    }
}
