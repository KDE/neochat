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

    title: "Room Settings - " + (room ? room.displayName : "")
    modal: true

    contentItem: ColumnLayout {
        RowLayout {
            Layout.fillWidth: true

            spacing: 16

            Avatar {
                Layout.preferredWidth: 72
                Layout.preferredHeight: 72
                Layout.alignment: Qt.AlignTop

                hint: room ? room.displayName : "No name"
                source: room ? room.avatarMediaId : null
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.margins: 4

                AutoTextField {
                    Layout.fillWidth: true

                    text: room ? room.name : ""
                    placeholderText: "Room Name"
                }

                AutoTextField {
                    Layout.fillWidth: true

                    text: room ? room.topic : ""
                    placeholderText: "Room Topic"
                }
            }
        }

        Control {
            Layout.fillWidth: true

            visible: room ? room.predecessorId : false

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

            visible: room ? room.successorId : false

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

                    model: room ? room.aliases : null

                    currentIndex: room ? room.aliases.indexOf(room.canonicalAlias) : -1
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Label {
                    Layout.preferredWidth: 100
                    Layout.alignment: Qt.AlignTop

                    wrapMode: Label.Wrap
                    text: "Aliases"
                    color: MPalette.lighter
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    Repeater {
                        model: room ? room.aliases : null

                        delegate: Label {
                            Layout.fillWidth: true

                            text: modelData

                            font.pixelSize: 12
                            color: MPalette.lighter
                        }
                    }
                }
            }
        }
    }

    onClosed: destroy()
}

