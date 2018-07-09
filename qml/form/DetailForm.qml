import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.4
import QtQuick.Controls.Material 2.4
import "qrc:/qml/component"

Item {
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Pane {
            Layout.fillWidth: true
            Layout.preferredHeight: 250
            padding: 32

            background: Rectangle {
                color: Material.accent
            }

            Column {
                anchors.fill: parent

                ImageStatus {
                    z: 10
                    width: 96
                    height: width
                    source: "qrc:/asset/img/avatar.png"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    text: "Astolfo"
                    color: "white"
                    font.pointSize: 28
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    text: "Rider of Black"
                    color: "#cdcdcd"
                    font.pointSize: 12
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Row {
                    height: 48
                    anchors.horizontalCenter: parent.horizontalCenter
                    ItemDelegate {
                        width: parent.height
                        height: parent.height

                        contentItem: MaterialIcon { icon: "\ue0b7" }
                    }

                    ItemDelegate {
                        width: parent.height
                        height: parent.height

                        contentItem: MaterialIcon { icon: "\ue62e" }
                    }
                }
            }
        }

        Pane {
            Layout.fillWidth: true
            Layout.fillHeight: true

            leftPadding: 96
            rightPadding: 96

            GridLayout {
                width: parent.width
                columns: 2
                flow: GridLayout.LeftToRight
                anchors.horizontalCenter: parent.horizontalCenter
                columnSpacing: 32

                Text {
                    text: "Matrix ID"
                }

                Text {
                    Layout.fillWidth: true
                    text: "Welcome"
                }

                Text {
                    text: "Status"
                }

                Text {
                    text: "Overline"
                }
            }
        }
    }
}
