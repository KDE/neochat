import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.3
import "qrc:/qml/component"

Item {
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

            background: Rectangle {
                color: "#eaeaea"
            }

            ListModel {
                id: listModel
                ListElement {
                    name: "Bill Smith"
                    number: "555 3264"
                }
                ListElement {
                    name: "John Brown"
                    number: "555 8426"
                }
                ListElement {
                    name: "Sam Wise"
                    number: "555 0473"
                }
                ListElement {
                    name: "Bill Smith"
                    number: "555 3264"
                }
                ListElement {
                    name: "John Brown"
                    number: "555 8426"
                }
                ListElement {
                    name: "Sam Wise"
                    number: "555 0473"
                }
                ListElement {
                    name: "Bill Smith"
                    number: "555 3264"
                }
                ListElement {
                    name: "John Brown"
                    number: "555 8426"
                }
                ListElement {
                    name: "Sam Wise"
                    number: "555 0473"
                }
                ListElement {
                    name: "Bill Smith"
                    number: "555 3264"
                }
                ListElement {
                    name: "John Brown"
                    number: "555 8426"
                }
                ListElement {
                    name: "Sam Wise"
                    number: "555 0473"
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

                    contentItem: Item {
                        Row {
                            spacing: 16

                            ImageStatus {
                                width: parent.height
                                height: parent.height
                                source: "qrc:/asset/img/avatar.png"
                            }

                            Column {
                                Text {
                                    text: name
                                    color: "#424242"
                                }
                                Text {
                                    text: number
                                    color: "#424242"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
