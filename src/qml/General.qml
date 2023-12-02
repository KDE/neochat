// SPDX-FileCopyrightText: 2019-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
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
    required property NeoChatConnection connection

    title: i18n("General")

    FormCard.FormHeader {
        title: i18n("Room Information")
    }
    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            background: null
            contentItem: RowLayout {
                Item {
                    Layout.fillWidth: true
                }
                KirigamiComponents.Avatar {
                    id: avatar
                    Layout.alignment: Qt.AlignRight
                    name: room.name
                    source: room.avatarMediaId
                    implicitWidth: Kirigami.Units.iconSizes.enormous
                    implicitHeight: Kirigami.Units.iconSizes.enormous
                }
                QQC2.Button {
                    Layout.alignment: Qt.AlignLeft
                    enabled: room.canSendState("m.room.avatar")
                    visible: enabled
                    icon.name: "cloud-upload"
                    text: i18n("Update avatar")
                    display: QQC2.AbstractButton.IconOnly

                    onClicked: {
                        const fileDialog = openFileDialog.createObject(QQC2.ApplicationWindow.overlay)

                        fileDialog.chosen.connect(function(path) {
                            if (!path) return

                            room.changeAvatar(path)
                        })

                        fileDialog.open()
                    }

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.visible: hovered
                }
                Item {
                    Layout.fillWidth: true
                }
            }
        }
        FormCard.FormTextFieldDelegate {
            id: roomNameField
            label: i18n("Room name:")
            text: room.name
            readOnly: !room.canSendState("m.room.name")
        }
        FormCard.AbstractFormDelegate {
            id: roomTopicField
            background: Item {}
            contentItem: ColumnLayout {
                QQC2.Label {
                    id: roomTopicLabel
                    text: i18n("Room topic:")
                    Layout.fillWidth: true
                }
                QQC2.TextArea {
                    id: roomTopicTextArea
                    Accessible.description: roomTopicLabel.text
                    Layout.fillWidth: true
                    wrapMode: TextEdit.Wrap
                    text: room.topic
                    readOnly: !room.canSendState("m.room.topic")
                    onTextChanged: roomTopicField.text = text
                }
            }
        }
        FormCard.AbstractFormDelegate {
            visible: !roomNameField.readOnly || !roomTopicTextArea.readOnly
            background: Item {}
            contentItem: RowLayout {
                Item {
                    Layout.fillWidth: true
                }
                QQC2.Button {
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                    Layout.topMargin: Kirigami.Units.smallSpacing
                    enabled: room.name !== roomNameField.text || room.topic !== roomTopicField.text
                    text: i18n("Save")
                    onClicked: {
                        if (room.name != roomNameField.text) {
                            room.setName(roomNameField.text)
                        }

                        if (room.topic != roomTopicField.text) {
                            room.setTopic(roomTopicField.text)
                        }
                    }
                }
            }
        }
        FormCard.FormTextDelegate {
            id: roomIdDelegate
            text: i18n("Room ID")
            description: room.id

            contentItem.children: QQC2.Button {
                visible: roomIdDelegate.hovered
                text: i18n("Copy room ID to clipboard")
                icon.name: "edit-copy"
                display: QQC2.AbstractButton.IconOnly

                onClicked: {
                    Clipboard.saveText(room.id)
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
            }
        }
        FormCard.FormTextDelegate {
            text: i18n("Room version")
            description: room.version

            contentItem.children: QQC2.Button {
                visible: room.canSwitchVersions()
                enabled: room.version < room.maxRoomVersion
                text: i18n("Upgrade Room")
                icon.name: "arrow-up-double"

                onClicked: {
                    if (room.canSwitchVersions()) {
                        roomUpgradeSheet.currentRoomVersion = room.version
                        roomUpgradeSheet.open()
                    }
                }

                QQC2.ToolTip {
                    text: text
                    delay: Kirigami.Units.toolTipDelay
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Aliases")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            visible: room.aliases.length <= 0
            text: i18n("No canonical alias set")
        }
        Repeater {
            id: altAliasRepeater
            model: room.aliases.slice().reverse()

            delegate: FormCard.FormTextDelegate {
                text: modelData
                description: room.canonicalAlias.length > 0 && modelData === room.canonicalAlias ? "Canonical alias" : ""
                contentItem.children: [
                    QQC2.ToolButton {
                        id: setCanonicalAliasButton
                        visible: modelData !== room.canonicalAlias && room.canSendState("m.room.canonical_alias")
                        text: i18n("Make this alias the room's canonical alias")
                        icon.name: "checkmark"
                        display: QQC2.AbstractButton.IconOnly

                        onClicked: {
                            room.setCanonicalAlias(modelData)
                        }
                        QQC2.ToolTip {
                            text: setCanonicalAliasButton.text
                            delay: Kirigami.Units.toolTipDelay
                        }
                    },
                    QQC2.ToolButton {
                        id: deleteButton
                        visible: room.canSendState("m.room.canonical_alias")
                        text: i18n("Delete alias")
                        icon.name: "edit-delete-remove"
                        display: QQC2.AbstractButton.IconOnly

                        onClicked: {
                            room.unmapAlias(modelData)
                        }
                        QQC2.ToolTip {
                            text: deleteButton.text
                            delay: Kirigami.Units.toolTipDelay
                        }
                    }
                ]

            }
        }
        FormCard.AbstractFormDelegate {
            visible: room.canSendState("m.room.canonical_alias")

            contentItem : RowLayout {
                Kirigami.ActionTextField {
                    id: aliasAddField

                    Layout.fillWidth: true

                    placeholderText: i18n("#new_alias:server.org")

                    rightActions: Kirigami.Action {
                        icon.name: "edit-clear"
                        visible: aliasAddField.text.length > 0
                        onTriggered: {
                            aliasAddField.text = ""
                        }
                    }

                    onAccepted: {
                        room.mapAlias(aliasAddField.text)
                    }
                }
                QQC2.Button {
                    id: addButton

                    text: i18n("Add new alias")
                    Accessible.name: text
                    icon.name: "list-add"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: aliasAddField.text.length > 0

                    onClicked: {
                        room.mapAlias(aliasAddField.text)
                    }

                    QQC2.ToolTip {
                        text: addButton.text
                        delay: Kirigami.Units.toolTipDelay
                    }
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("URL Previews")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: i18n("Enable URL previews by default for room members")
            checked: room.defaultUrlPreviewState
            visible: room.canSendState("org.matrix.room.preview_urls")
            onToggled: {
                room.defaultUrlPreviewState = checked
            }
        }
        FormCard.FormCheckDelegate {
            text: i18n("Enable URL previews")
            // Most users won't see the above setting so tell them the default.
            description: room.defaultUrlPreviewState ? i18n("URL previews are enabled by default in this room") : i18n("URL previews are disabled by default in this room")
            checked: room.urlPreviewEnabled
            onToggled: {
                room.urlPreviewEnabled = checked
            }
        }
    }
    FormCard.FormHeader {
        title: i18n("Official Parent Spaces")
    }
    FormCard.FormCard {
        Repeater {
            id: officalParentRepeater
            model: root.room.parentIds

            delegate: FormCard.FormTextDelegate {
                id: officalParentDelegate
                required property string modelData
                property NeoChatRoom space: root.connection.room(modelData)
                text: {
                    if (space) {
                        return space.displayName;
                    } else {
                        return modelData;
                    }
                }
                description: {
                    if (space) {
                        if (space.canonicalAlias.length > 0) {
                            return space.canonicalAlias;
                        } else {
                            return modelData;
                        }
                    } else {
                        return "";
                    }
                }

                contentItem.children: RowLayout {
                    QQC2.Label {
                        visible: root.room.canonicalParent === officalParentDelegate.modelData
                        text: i18n("Canonical")
                    }
                    QQC2.ToolButton {
                        visible: root.room.canSendState("m.space.parent") && root.room.canonicalParent !== officalParentDelegate.modelData
                        display: QQC2.AbstractButton.IconOnly
                        action: Kirigami.Action {
                            id: canonicalParentAction
                            text: i18n("Make canonical parent")
                            icon.name: "checkmark"
                            onTriggered: root.room.canonicalParent = officalParentDelegate.modelData
                        }
                        QQC2.ToolTip {
                            text: canonicalParentAction.text
                            delay: Kirigami.Units.toolTipDelay
                        }
                    }
                    QQC2.ToolButton {
                        visible: officalParentDelegate?.space.canSendState("m.space.child") && root.room.canSendState("m.space.parent")
                        display: QQC2.AbstractButton.IconOnly
                        action: Kirigami.Action {
                            id: removeParentAction
                            text: i18n("Remove parent")
                            icon.name: "edit-delete-remove"
                            onTriggered: root.room.removeParent(officalParentDelegate.modelData)
                        }
                        QQC2.ToolTip {
                            text: removeParentAction.text
                            delay: Kirigami.Units.toolTipDelay
                        }
                    }
                }
            }
        }
        FormCard.FormTextDelegate {
            visible: officalParentRepeater.count <= 0
            text: i18n("This room has no official parent spaces.")
        }
        FormCard.FormButtonDelegate {
            visible: root.room.canSendState("m.space.parent")
            text: i18nc("@action:button", "Add new official parent")
            onClicked: selectParentDialog.createObject(applicationWindow().overlay).open();

            Component {
                id: selectParentDialog
                SelectParentDialog {
                    room: root.room
                }
            }
        }
    }

    Kirigami.InlineMessage {
        Layout.maximumWidth: Kirigami.Units.gridUnit * 30
        Layout.alignment: Qt.AlignHCenter
        text: i18n("This room continues another conversation.")
        type: Kirigami.MessageType.Information
        visible: room.predecessorId && room.connection.room(room.predecessorId)
        actions: Kirigami.Action {
            text: i18n("See older messages…")
            onTriggered: {
                RoomManager.enterRoom(root.connection.room(room.predecessorId));
                root.close();
            }
        }
    }
    Kirigami.InlineMessage {
        Layout.maximumWidth: Kirigami.Units.gridUnit * 30
        Layout.alignment: Qt.AlignHCenter
        text: i18n("This room has been replaced.")
        type: Kirigami.MessageType.Information
        visible: room.successorId && room.connection.room(room.successorId)
        actions: Kirigami.Action {
            text: i18n("See new room…")
            onTriggered: {
                RoomManager.enterRoom(root.connection.room(room.successorId));
                root.close();
            }
        }
    }

    property Component openFileDialog: Component {
        id: openFileDialog

        OpenFileDialog {
            parentWindow: root.Window.window
        }
    }

    property Kirigami.OverlaySheet roomUpgradeSheet: Kirigami.OverlaySheet {
        id: roomUpgradeSheet

        property var currentRoomVersion

        title: i18n("Upgrade the Room")
        Kirigami.FormLayout {
            QQC2.SpinBox {
                id: spinBox
                Kirigami.FormData.label: i18n("Select new version")
                from: room.version
                to: room.maxRoomVersion
                value: room.version
            }
            QQC2.Button {
                text: i18n("Confirm")
                onClicked: {
                    room.switchVersion(spinBox.value)
                    roomUpgradeSheet.close()
                }
            }
        }
    }
}

