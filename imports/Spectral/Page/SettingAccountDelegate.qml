import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import Spectral.Component 2.0

import Spectral 0.1
import Spectral.Setting 0.1

Column {
    property bool expanded: false

    spacing: 8

    ItemDelegate {
        width: accountSettingsListView.width
        height: 64

        Row {
            anchors.fill: parent
            anchors.margins: 8

            spacing: 8

            ImageItem {
                width: parent.height
                height: parent.height

                hint: user.displayName
                image: user.avatar
            }

            ColumnLayout {
                Label {
                    text: user.displayName
                }
                Label {
                    text: user.id
                }
            }
        }

        onClicked: expanded = !expanded
    }

    ColumnLayout {
        width: parent.width - 32
        height: expanded ? implicitHeight : 0
        anchors.horizontalCenter: parent.horizontalCenter

        spacing: 0

        clip: true

        ListView {
            Layout.fillWidth: true
            Layout.preferredHeight: 24

            orientation: ListView.Horizontal

            spacing: 8

            model: ["#498882", "#42a5f5", "#5c6bc0", "#7e57c2", "#ab47bc", "#ff7043"]

            delegate: Rectangle {
                width: parent.height
                height: parent.height
                radius: width / 2

                color: modelData

                MouseArea {
                    anchors.fill: parent

                    onClicked: spectralController.setColor(connection.localUserId, modelData)
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "Homeserver:"
            }
            TextField {
                Layout.fillWidth: true

                text: connection.homeserver
                selectByMouse: true
                readOnly: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 16

            Label {
                text: "Device ID:"
            }
            TextField {
                Layout.fillWidth: true

                text: connection.deviceId
                selectByMouse: true
                readOnly: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 16

            Label {
                text: "Access Token:"
            }
            TextField {
                Layout.fillWidth: true

                text: connection.accessToken
                selectByMouse: true
                readOnly: true
            }
        }

        Button {
            Layout.fillWidth: true

            highlighted: true
            text: "Logout"

            onClicked: spectralController.logout(connection)
        }
    }
}
