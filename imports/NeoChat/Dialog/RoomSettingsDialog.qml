import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami

import NeoChat.Component 2.0
import NeoChat.Effect 2.0
import NeoChat.Setting 0.1

import org.kde.neochat 0.1

Dialog {
    property var room

    readonly property bool canChangeAvatar: room.canSendState("m.room.avatar")
    readonly property bool canChangeName: room.canSendState("m.room.name")
    readonly property bool canChangeTopic: room.canSendState("m.room.topic")
    readonly property bool canChangeCanonicalAlias: room.canSendState("m.room.canonical_alias")

    anchors.centerIn: parent

    width: 480
    height: window.height * 0.9

    id: root

    title: "Room Settings - " + room.displayName
    modal: true

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

                RippleEffect {
                    anchors.fill: parent

                    circular: true

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

            ColumnLayout {
                Layout.fillWidth: true
                Layout.margins: 4

                AutoTextField {
                    Layout.fillWidth: true

                    id: roomNameField

                    text: room.name
                    placeholderText: "Room Name"

                    enabled: canChangeName
                }

                AutoTextField {
                    Layout.fillWidth: true

                    id: roomTopicField

                    text: room.topic
                    placeholderText: "Room Topic"

                    enabled: canChangeTopic
                }
            }
        }

        Button {
            Layout.alignment: Qt.AlignRight

            visible: canChangeName || canChangeTopic

            text: "Save"
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

        MenuSeparator {
            Layout.fillWidth: true
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: scrollview

            clip: true

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ColumnLayout {
                width: scrollview.width

                Control {
                    Layout.fillWidth: true

                    visible: room.predecessorId && room.connection.room(room.predecessorId)

                    padding: 8

                    contentItem: RowLayout {
                        ColumnLayout {
                            Layout.fillWidth: true

                            spacing: 0

                            Label {
                                Layout.fillWidth: true

                                font.bold: true
                                color: MPalette.foreground
                                text: "This room continues another conversation."
                            }

                            Label {
                                Layout.fillWidth: true

                                color: MPalette.lighter
                                text: "Click here to see older messages."
                            }
                        }
                    }

                    background: Rectangle {
                        color: MPalette.banner

                        RippleEffect {
                            anchors.fill: parent

                            onClicked: {
                                roomListForm.enteredRoom = Controller.connection.room(room.predecessorId)
                                root.close()
                            }
                        }
                    }
                }

                Control {
                    Layout.fillWidth: true

                    visible: room.successorId && room.connection.room(room.successorId)

                    padding: 8

                    contentItem: RowLayout {
                        ColumnLayout {
                            Layout.fillWidth: true

                            spacing: 0

                            Label {
                                Layout.fillWidth: true

                                font.bold: true
                                color: MPalette.foreground
                                text: "This room has been replaced."
                            }

                            Label {
                                Layout.fillWidth: true

                                color: MPalette.lighter
                                text: "The conversation continues here."
                            }
                        }
                    }

                    background: Rectangle {
                        color: MPalette.banner

                        RippleEffect {
                            anchors.fill: parent

                            onClicked: {
                                roomListForm.enteredRoom = Controller.connection.room(room.successorId)
                                root.close()
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        Layout.preferredWidth: 100

                        wrapMode: Label.Wrap
                        text: "Canonical Alias"
                        color: MPalette.lighter
                    }

                    ComboBox {
                        Layout.fillWidth: true

                        id: canonicalAliasComboBox

                        enabled: canChangeCanonicalAlias

                        model: room.aliases

                        currentIndex: room.aliases.indexOf(room.canonicalAlias)
                        onCurrentIndexChanged: {
                            if (room.canonicalAlias != room.aliases[currentIndex]) {
                                room.setCanonicalAlias(room.aliases[currentIndex])
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    visible: room.altAliases && room.altAliases.length

                    Label {
                        Layout.preferredWidth: 100
                        Layout.alignment: Qt.AlignTop

                        wrapMode: Label.Wrap
                        text: "Alt Aliases"
                        color: MPalette.lighter
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        spacing: 0

                        Repeater {
                            model: room.altAliases

                            delegate: RowLayout {
                                Layout.maximumWidth: parent.width

                                Label {
                                    text: modelData

                                    font.pixelSize: 12
                                    color: MPalette.lighter
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
    }

    Component {
        id: openFileDialog

        OpenFileDialog {}
    }

    onClosed: destroy()
}

