// SPDX-FileCopyrightText: 2019-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

pragma ComponentBehavior: Bound

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

    title: i18nc("@title", "General")

    KirigamiComponents.Avatar {
        id: avatar
        Layout.alignment: Qt.AlignHCenter
        Layout.topMargin: Kirigami.Units.gridUnit
        name: root.room.name
        source: root.room.avatarMediaUrl
        implicitWidth: Kirigami.Units.iconSizes.enormous
        implicitHeight: Kirigami.Units.iconSizes.enormous

        QQC2.Button {
            enabled: root.room.canSendState("m.room.avatar")
            visible: enabled
            icon.name: "cloud-upload"
            text: i18nc("@action:button", "Update avatar")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const fileDialog = openFileDialog.createObject(QQC2.Overlay.overlay) as OpenFileDialog;
                fileDialog.chosen.connect(function (path) {
                    if (!path)
                        return;
                    room.changeAvatar(path);
                });
                fileDialog.open();
            }

            anchors {
                bottom: parent.bottom
                right: parent.right
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.gridUnit

        FormCard.FormTextFieldDelegate {
            id: roomNameField
            label: i18nc("@label:textbox Room name", "Name:")
            text: root.room.name
            readOnly: !root.room.canSendState("m.room.name")
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextAreaDelegate {
            id: roomTopicField
            label: i18nc("@label:textobx Room topic", "Topic:")
            text: root.room.topic
            readOnly: !root.room.canSendState("m.room.topic")
            onTextChanged: roomTopicField.text = text
        }

        FormCard.FormDelegateSeparator {
            visible: !roomNameField.readOnly || !roomTopicField.readOnly
        }

        FormCard.FormButtonDelegate {
            visible: !roomNameField.readOnly || !roomTopicField.readOnly
            text: i18nc("@action:button", "Save")
            icon.name: "document-save-symbolic"
            onClicked: {
                if (root.room.name != roomNameField.text) {
                    root.room.setName(roomNameField.text);
                }
                if (root.room.topic != roomTopicField.text) {
                    root.room.setTopic(roomTopicField.text);
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title", "Aliases")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            visible: root.room.aliases.length <= 0
            text: i18nc("@info", "No canonical alias set")
        }
        Repeater {
            id: altAliasRepeater
            model: root.room.aliases.slice().reverse()

            delegate: FormCard.FormTextDelegate {
                id: altAliasDelegate
                required property string modelData
                text: modelData
                description: root.room.canonicalAlias.length > 0 && modelData === root.room.canonicalAlias ? "Canonical alias" : ""
                textItem.textFormat: Text.PlainText

                contentItem.children: [
                    QQC2.ToolButton {
                        id: setCanonicalAliasButton
                        visible: altAliasDelegate.modelData !== root.room.canonicalAlias && root.room.canSendState("m.room.canonical_alias")
                        text: i18nc("@action:button", "Make this alias the room's canonical alias")
                        icon.name: "checkmark"
                        display: QQC2.AbstractButton.IconOnly

                        onClicked: root.room.setCanonicalAlias(altAliasDelegate.modelData)

                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        QQC2.ToolTip.visible: hovered
                    },
                    QQC2.ToolButton {
                        id: deleteButton
                        visible: root.room.canSendState("m.room.canonical_alias")
                        text: i18nc("@action:button", "Delete alias")
                        icon.name: "edit-delete-remove"
                        display: QQC2.AbstractButton.IconOnly

                        onClicked: root.room.unmapAlias(altAliasDelegate.modelData)

                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        QQC2.ToolTip.visible: hovered
                    }
                ]
            }
        }
        FormCard.AbstractFormDelegate {
            visible: root.room.canSendState("m.room.canonical_alias")

            contentItem: RowLayout {
                Kirigami.ActionTextField {
                    id: aliasAddField

                    Layout.fillWidth: true

                    placeholderText: i18nc("The new alias (room address) to be added to a room", "#new_alias:server.org")

                    rightActions: Kirigami.Action {
                        icon.name: "edit-clear"
                        visible: aliasAddField.text.length > 0
                        onTriggered: {
                            aliasAddField.text = "";
                        }
                    }

                    onAccepted: {
                        root.room.mapAlias(aliasAddField.text);
                    }
                }
                QQC2.Button {
                    id: addButton

                    text: i18nc("@action:button", "Add new alias")
                    Accessible.name: text
                    icon.name: "list-add"
                    display: QQC2.AbstractButton.IconOnly
                    enabled: aliasAddField.text.length > 0

                    onClicked: root.room.mapAlias(aliasAddField.text)

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title", "URL Previews")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: i18nc("@label:checkbox", "Enable URL previews by default for room members")
            checked: root.room.defaultUrlPreviewState
            visible: root.room.canSendState("org.matrix.room.preview_urls")
            onToggled: {
                root.room.defaultUrlPreviewState = checked;
            }
        }
        FormCard.FormCheckDelegate {
            enabled: NeoChatConfig.showLinkPreview
            text: i18nc("@label:checkbox", "Enable URL previews")
            // Most users won't see the above setting so tell them the default.
            description: root.room.defaultUrlPreviewState ? i18nc("@info", "URL previews are enabled by default in this room") : i18nc("@info", "URL previews are disabled by default in this room")
            checked: root.room.urlPreviewEnabled
            onToggled: {
                root.room.urlPreviewEnabled = checked;
            }
        }
    }
    Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.maximumWidth: Kirigami.Units.gridUnit * 30
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.alignment: Qt.AlignHCenter
        text: i18nc("As in the user has switched off showing previews of hyperlinks in timeline messages", "URL previews are currently disabled for your account")
        type: Kirigami.MessageType.Information
        visible: !NeoChatConfig.showLinkPreview
        actions: Kirigami.Action {
            text: i18nc("@action:button", "Enable")
            onTriggered: {
                NeoChatConfig.showLinkPreview = true;
                NeoChatConfig.save();
            }
        }
    }
    FormCard.FormHeader {
        title: i18nc("@title", "Official Parent Spaces")
    }
    FormCard.FormCard {
        Repeater {
            id: officalParentRepeater
            model: root.room.parentIds

            delegate: FormCard.FormTextDelegate {
                id: officalParentDelegate
                required property string modelData
                property NeoChatRoom space: root.connection.room(modelData) as NeoChatRoom
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
                        text: i18nc("@action:button", "Canonical")
                    }
                    QQC2.ToolButton {
                        visible: root.room.canSendState("m.space.parent") && root.room.canonicalParent !== officalParentDelegate.modelData
                        display: QQC2.AbstractButton.IconOnly
                        text: i18nc("@action:button", "Make canonical parent")
                        icon.name: "checkmark"
                        onClicked: root.room.canonicalParent = officalParentDelegate.modelData

                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        QQC2.ToolTip.visible: hovered
                    }
                    QQC2.ToolButton {
                        visible: officalParentDelegate?.space.canSendState("m.space.child") && root.room.canSendState("m.space.parent")
                        display: QQC2.AbstractButton.IconOnly
                        text: i18nc("@action:button", "Remove parent")
                        icon.name: "edit-delete-remove"
                        onClicked: root.room.removeParent(officalParentDelegate.modelData)

                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                }
            }
        }
        FormCard.FormTextDelegate {
            visible: officalParentRepeater.count <= 0
            text: i18nc("@info", "This room has no official parent spaces.")
        }
        FormCard.FormButtonDelegate {
            visible: root.room.canSendState("m.space.parent")
            text: i18nc("@action:button", "Add new official parent")
            onClicked: (selectParentDialog.createObject(QQC2.Overlay.overlay) as SelectParentDialog).open()

            Component {
                id: selectParentDialog
                SelectParentDialog {
                    room: root.room
                }
            }
        }
    }

    Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.maximumWidth: Kirigami.Units.gridUnit * 30
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.alignment: Qt.AlignHCenter
        text: i18nc("@info", "This room continues another conversation.")
        type: Kirigami.MessageType.Information
        visible: root.room.predecessorId
        actions: Kirigami.Action {
            text: i18nc("@action:button", "See older messages…")
            onTriggered: {
                RoomManager.resolveResource(root.room.predecessorId);
                root.close();
            }
        }
    }
    Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.maximumWidth: Kirigami.Units.gridUnit * 30
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.alignment: Qt.AlignHCenter
        text: i18nc("@info", "This room has been replaced.")
        type: Kirigami.MessageType.Information
        visible: root.room.successorId
        actions: Kirigami.Action {
            text: i18nc("@action:button", "See new room…")
            onTriggered: {
                RoomManager.resolveResource(root.room.successorId);
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
}
