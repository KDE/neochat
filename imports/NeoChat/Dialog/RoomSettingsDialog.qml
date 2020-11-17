/**
 * SPDX-FileCopyrightText: 2019-2020 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami

import NeoChat.Component 1.0
import NeoChat.Effect 1.0
import NeoChat.Setting 1.0

import org.kde.neochat 1.0

Kirigami.OverlaySheet {
    id: root

    property var room

    readonly property bool canChangeAvatar: room.canSendState("m.room.avatar")
    readonly property bool canChangeName: room.canSendState("m.room.name")
    readonly property bool canChangeTopic: room.canSendState("m.room.topic")
    readonly property bool canChangeCanonicalAlias: room.canSendState("m.room.canonical_alias")

    parent: applicationWindow().overlay


    header: Kirigami.Heading {
        text: i18nc("%1 is the room name", "Room Settings - %1", room.displayName)
    }

    contentItem: ColumnLayout {
        RowLayout {
            Layout.fillWidth: true

            spacing: 16

            Kirigami.Avatar {
                Layout.preferredWidth: 72
                Layout.preferredHeight: 72
                Layout.alignment: Qt.AlignTop

                name: room.displayName
                source: room.avatarMediaId ? "image://mxc/" + room.avatarMediaId : ""

                MouseArea {
                    anchors.fill: parent

                    enabled: canChangeAvatar

                    onClicked: {
                        var fileDialog = openFileDialog.createObject(ApplicationWindow.overlay)

                        fileDialog.chosen.connect(function(path) {
                            if (!path) return

                            room.changeAvatar(path)
                        })

                        fileDialog.open()
                    }
                }
            }

            Kirigami.FormLayout {
                Layout.fillWidth: true

                TextField {
                    id: roomNameField
                    text: room.name
                    Kirigami.FormData.label: i18n("Room Name:")
                    enabled: canChangeName
                }

                TextField {
                    id: roomTopicField
                    Layout.fillWidth: true
                    text: room.topic
                    Kirigami.FormData.label: i18n("Room topic:")
                    enabled: canChangeTopic
                }
                Button {
                    Layout.alignment: Qt.AlignRight

                    visible: canChangeName || canChangeTopic

                    text: i18n("Save")
                    highlighted: true

                    onClicked: {
                        if (room.name != roomNameField.text) {
                            room.setName(roomNameField.text)
                        }

                        if (room.topic != roomTopicField.text) {
                            room.setTopic(roomTopicField.text)
                        }
                    }
                }

                Kirigami.Separator {}

                ComboBox {
                    id: canonicalAliasComboBox
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
                    Kirigami.FormData.label: i18n("Alt Aliases")
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
                                    color: Kirigami.Theme.disabledColor
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
}

