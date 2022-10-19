// SPDX-FileCopyrightText: 2019-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root

    property var room

    readonly property bool canChangeAvatar: room.canSendState("m.room.avatar")
    readonly property bool canChangeName: room.canSendState("m.room.name")
    readonly property bool canChangeTopic: room.canSendState("m.room.topic")
    readonly property bool canChangeCanonicalAlias: room.canSendState("m.room.canonical_alias")

    title: i18n("General")

    ColumnLayout {
        Kirigami.FormLayout {
            Layout.fillWidth: true

            Kirigami.Avatar {
                Layout.bottomMargin: Kirigami.Units.largeSpacing

                name: room.name
                source: room.avatarMediaId ? ("image://mxc/" + room.avatarMediaId) : ""

                RoundButton {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: Kirigami.Units.gridUnits
                    width: Kirigami.Units.gridUnits
                    icon.name: 'cloud-upload'
                    Accessible.name: i18n("Update avatar")
                    enabled: canChangeAvatar
                    onClicked: {
                        const fileDialog = openFileDialog.createObject(ApplicationWindow.overlay)

                        fileDialog.chosen.connect(function(path) {
                            if (!path) return

                            room.changeAvatar(path)
                        })

                        fileDialog.open()
                    }
                }
            }
            TextField {
                id: roomNameField
                text: room.name
                Kirigami.FormData.label: i18n("Room Name:")
                enabled: canChangeName
            }

            TextArea {
                id: roomTopicField
                Layout.fillWidth: true
                text: room.topic
                Kirigami.FormData.label: i18n("Room topic:")
                enabled: canChangeTopic
            }


            Kirigami.Separator {
                Layout.fillWidth: true
                visible: canonicalAliasComboBox.visible || altAlias.visible
            }

            ComboBox {
                id: canonicalAliasComboBox
                visible: room.aliases && room.aliases.length
                Kirigami.FormData.label: i18n("Canonical Alias:")
                popup.z: 999; // HACK This is an absolute hack, but combos inside OverlaySheets have their popups show up underneath, because of fun z ordering stuff

                enabled: canChangeCanonicalAlias

                model: room.aliases

                currentIndex: room.aliases.indexOf(room.canonicalAlias)
                onCurrentIndexChanged: {
                    if (room.canonicalAlias != room.aliases[currentIndex]) {
                        room.setCanonicalAlias(room.aliases[currentIndex])
                    }
                }
            }

            RowLayout {
                id: altAlias
                Kirigami.FormData.label: i18n("Other Aliases:")
                Layout.fillWidth: true

                visible: room.altAliases && room.altAliases.length

                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: 0

                    Repeater {
                        model: room.altAliases

                        delegate: RowLayout {
                            Layout.maximumWidth: parent.width

                            Label {
                                text: modelData
                            }

                            ToolButton {
                                icon.name: ""
                                onClicked: room.removeLocalAlias(modelData)
                            }
                        }
                    }
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
            visible: next.visible || prev.visible 
        }

        Control {
            id: next
            Layout.fillWidth: true

            visible: room.predecessorId && room.connection.room(room.predecessorId)

            padding: Kirigami.Units.largeSpacing

            contentItem: Kirigami.InlineMessage {
                text: i18n("This room continues another conversation.")
                actions: Kirigami.Action {
                    text: i18n("See older messages...")
                    onTriggered: {
                        roomListForm.enteredRoom = Controller.activeConnection.room(room.predecessorId)
                        root.close()
                    }
                }
            }
        }

        Control {
            id: prev
            Layout.fillWidth: true

            visible: room.successorId && room.connection.room(room.successorId)

            padding: Kirigami.Units.largeSpacing

            contentItem: Kirigami.InlineMessage {
                text: i18n("This room has been replaced.")
                actions: Kirigami.Action {
                    text: i18n("See new room...")
                    onTriggered: {
                        roomListForm.enteredRoom = Controller.activeConnection.room(room.successorId)
                        root.close()
                    }
                }
            }
        }

        Component {
            id: openFileDialog

            OpenFileDialog {}
        }
    }

    footer: ToolBar {
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }
            Button {
                Layout.alignment: Qt.AlignRight
                enabled: room.name !== roomNameField.text || room.topic !== roomTopicField.text
                text: i18n("Apply")
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
}

