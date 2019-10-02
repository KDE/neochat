import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Spectral.Component 2.0
import Spectral.Effect 2.0
import Spectral.Setting 0.1

Dialog {
    property var room

    anchors.centerIn: parent
    width: 480

    id: root

    title: "Room Settings - " + room.displayName
    modal: true

    contentItem: ColumnLayout {
        RowLayout {
            Layout.fillWidth: true

            spacing: 16

            Avatar {
                Layout.preferredWidth: 72
                Layout.preferredHeight: 72
                Layout.alignment: Qt.AlignTop

                hint: room.displayName
                source: room.avatarMediaId

                RippleEffect {
                    anchors.fill: parent

                    circular: true

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
                }

                AutoTextField {
                    Layout.fillWidth: true

                    id: roomTopicField

                    text: room.topic
                    placeholderText: "Room Topic"
                }
            }
        }

        Control {
            Layout.fillWidth: true

            visible: room.predecessorId && room.connection.room(room.predecessorId)

            padding: 8

            contentItem: RowLayout {
                MaterialIcon {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    icon: "\ue8d4"
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: 0

                    Label {
                        Layout.fillWidth: true

                        font.bold: true
                        color: MPalette.foreground
                        text: "This room is a continuation of another conversation."
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
                        roomListForm.enteredRoom = spectralController.connection.room(room.predecessorId)
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
                MaterialIcon {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    icon: "\ue8d4"
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: 0

                    Label {
                        Layout.fillWidth: true

                        font.bold: true
                        color: MPalette.foreground
                        text: "This room has been replaced and is no longer active."
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
                        roomListForm.enteredRoom = spectralController.connection.room(room.successorId)
                        root.close()
                    }
                }
            }
        }

        Button {
            Layout.alignment: Qt.AlignRight

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

        ColumnLayout {
            Layout.fillWidth: true

            RowLayout {
                Layout.fillWidth: true

                Label {
                    Layout.preferredWidth: 100

                    wrapMode: Label.Wrap
                    text: "Main Alias"
                    color: MPalette.lighter
                }

                ComboBox {
                    Layout.fillWidth: true

                    id: canonicalAliasComboBox

                    model: room.remoteAliases

                    currentIndex: room.remoteAliases.indexOf(room.canonicalAlias)
                    onCurrentIndexChanged: {
                        if (room.canonicalAlias != room.remoteAliases[currentIndex]) {
                            room.setCanonicalAlias(room.remoteAliases[currentIndex])
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Label {
                    Layout.preferredWidth: 100
                    Layout.alignment: Qt.AlignTop

                    wrapMode: Label.Wrap
                    text: "Local Aliases"
                    color: MPalette.lighter
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: 0

                    Repeater {
                        model: room.localAliases

                        delegate: RowLayout {
                            Layout.maximumWidth: parent.width

                            Label {
                                text: modelData

                                font.pixelSize: 12
                                color: MPalette.lighter
                            }

                            MaterialIcon {
                                icon: "\ue5cd"

                                color: MPalette.lighter
                                font.pixelSize: 12

                                RippleEffect {
                                    anchors.fill: parent

                                    circular: true

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

