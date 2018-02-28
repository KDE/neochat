import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.3
import QtGraphicalEffects 1.0
import "qrc:/qml/component"

Item {
    ColumnLayout {
        anchors.fill: parent

        Pane {
            z: 10
            padding: 16

            Layout.fillWidth: true
            Layout.preferredHeight: 80

            background: Rectangle {
                color: "#eaeaea"
            }

            Row {
                anchors.fill: parent
                spacing: 16

                ImageStatus {
                    width: parent.height
                    height: parent.height
                    source: "qrc:/asset/img/avatar.png"
                }

                Column {
                    height: parent.height
                    Text {
                        text: "Astolfo"
                        font.pointSize: 18
                        color: "#424242"
                    }
                    Text {
                        text: "Rider of Black"
                        color: "#424242"
                    }
                }
            }
        }

        Pane {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Pane {
            z: 10
            padding: 16

            Layout.fillWidth: true
            Layout.preferredHeight: 80

            RowLayout {
                anchors.fill: parent
                spacing: 0

                ItemDelegate {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    contentItem: Text {
                        text: "\ue226"
                        color: "#424242"
                        font.pointSize: 16
                        font.family: materialFont.name
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                TextField {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    placeholderText: "Send a Message"
                    leftPadding: 16
                    topPadding: 0
                    bottomPadding: 0

                    background: Rectangle {
                        color: "#eaeaea"
                    }
                }

                ItemDelegate {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    contentItem: Text {
                        text: "\ue24e"
                        color: parent.pressed ? Material.accent : "#424242"
                        font.pointSize: 16
                        font.family: materialFont.name
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: "#eaeaea"
                    }
                }

                ItemDelegate {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    contentItem: Text {
                        text: "\ue163"
                        color: "#424242"
                        font.pointSize: 16
                        font.family: materialFont.name
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
