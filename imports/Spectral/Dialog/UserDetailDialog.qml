import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Spectral.Component 2.0
import Spectral.Effect 2.0
import Spectral.Setting 0.1

Dialog {
    property var room
    property var user

    anchors.centerIn: parent
    width: 360

    id: root

    modal: true

    contentItem: ColumnLayout {
        RowLayout {
            Layout.fillWidth: true

            spacing: 16

            Avatar {
                Layout.preferredWidth: 72
                Layout.preferredHeight: 72

                hint: user ? user.displayName : "No name"
                source: user ? user.avatarMediaId : null
            }

            ColumnLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true

                    font.pixelSize: 18
                    font.bold: true

                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                    text: user ? user.displayName : "No Name"
                    color: MPalette.foreground
                }

                Label {
                    Layout.fillWidth: true

                    text: "Online"
                    color: MPalette.lighter
                }
            }
        }

        MenuSeparator {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 8

            MaterialIcon {
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                Layout.alignment: Qt.AlignTop

                icon: "\ue88f"
                color: MPalette.lighter
            }

            ColumnLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true

                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                    text: user ? user.id : "No ID"
                    color: MPalette.accent
                }

                Label {
                    Layout.fillWidth: true

                    wrapMode: Label.Wrap
                    text: "User ID"
                    color: MPalette.lighter
                }
            }
        }

        MenuSeparator {
            Layout.fillWidth: true
        }

        Control {
            Layout.fillWidth: true

            contentItem: RowLayout {
                MaterialIcon {
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    Layout.alignment: Qt.AlignTop

                    icon: room.connection.isIgnored(user) ? "\ue7f5" : "\ue7f6"
                    color: MPalette.lighter
                }

                Label {
                    Layout.fillWidth: true

                    wrapMode: Label.Wrap
                    text: room.connection.isIgnored(user) ? "Unignore this user" : "Ignore this user"

                    color: MPalette.accent
                }
            }

            background: RippleEffect {
                onPrimaryClicked: {
                    root.close()
                    room.connection.isIgnored(user) ? room.connection.removeFromIgnoredUsers(user) : room.connection.addToIgnoredUsers(user)
                }
            }
        }

        Control {
            Layout.fillWidth: true

            contentItem: RowLayout {
                MaterialIcon {
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    Layout.alignment: Qt.AlignTop

                    icon: "\ue5d9"
                    color: MPalette.lighter
                }

                Label {
                    Layout.fillWidth: true

                    wrapMode: Label.Wrap
                    text: "Kick this user"

                    color: MPalette.accent
                }
            }

            background: RippleEffect {
                onPrimaryClicked: room.kickMember(user.id)
            }
        }
    }

    onClosed: destroy()
}

