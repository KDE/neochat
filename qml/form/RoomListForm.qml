import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.3
import "qrc:/qml/component"

Item {
    property var listModel
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Pane {
            z: 10
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            background: Rectangle {
                color: Qt.tint(Material.accent, "#20FFFFFF")
            }

            TextField {
                id: serverField
                width: parent.width
                height: 36
                leftPadding: 16
                topPadding: 0
                bottomPadding: 0
                anchors.verticalCenter: parent.verticalCenter

                background: Item {
                    Row {
                        anchors.fill: parent

                        Text {
                            width: parent.height
                            height: parent.height
                            text: "\ue8b6"
                            font.pointSize: 16
                            font.family: materialFont.name
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            height: parent.height
                            text: "Search"
                            color: "white"
                            font.pointSize: 12
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Rectangle {
                        width: serverField.activeFocus || serverField.text != "" ? parent.width : 0
                        height: parent.height
                        color: "white"

                        Behavior on width {
                            PropertyAnimation { easing.type: Easing.InOutQuad; duration: 200 }
                        }
                    }
                }
            }
        }

        Pane {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 0

            background: Item {
                anchors.fill: parent
                Rectangle {
                    anchors.fill: parent
                    color: "#eaeaea"
                }

                Text {
                    z: 10
                    text: "Here? No, not here."
                    anchors.centerIn: parent
                    visible: listView.count === 0
                }
            }

            ListView {
                id: listView
                width: parent.width
                height: parent.height

                model: listModel

                highlight: Rectangle {
                    color: Material.accent
                    opacity: 0.2
                }

                ScrollBar.vertical: ScrollBar { id: scrollBar }

                delegate: ItemDelegate {
                    width: parent.width
                    height: 80
                    onClicked: listView.currentIndex = index

                    contentItem:  Row {
                        width: parent.width - 32
                        height: parent.height - 32
                        spacing: 16

                        ImageStatus {
                            width: parent.height
                            height: parent.height
                            source: "qrc:/asset/img/avatar.png"
                        }

                        Column {
                            width: parent.width - parent.height - parent.spacing
                            height: parent.height
                            Text {
                                width: parent.width
                                text: name
                                color: "#424242"
                                font.pointSize: 16
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                            }
                            Text {
                                width: parent.width
                                text: value
                                color: "#424242"
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                            }
                        }
                    }
                }
            }
        }
    }
}
