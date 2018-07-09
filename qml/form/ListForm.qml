import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.4
import QtQml.Models 2.4
import Matrique 0.1

import "qrc:/qml/component"

Item {
    property alias listModel: delegateModel.model
    property alias currentIndex: listView.currentIndex
    readonly property bool mini: width <= 80 // Used as an indicator of whether the listform should be displayed as "Mini mode".

    DelegateModel {
        id: delegateModel
        groups: [
            DelegateModelGroup {
                name: "filterGroup"; includeByDefault: true
            }
        ]
        filterOnGroup: "filterGroup"

        delegate: ItemDelegate {
            width: parent.width
            height: 80
            onClicked: listView.currentIndex = index

            ToolTip.visible: pressed
            ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
            ToolTip.text: name

            contentItem:  RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                ImageStatus {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    source: avatar ? "image://mxc/" + avatar : ""
                    displayText: name
                    opaqueBackground: true
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignHCenter

                    visible: parent.width > 80

                    Label {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        text: {
                            if (name) {
                                return name;
                            }
                            if (alias) {
                                return alias;
                            }
                            return id
                        }
                        font.pointSize: 16
                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        text: topic ? topic : "No topic yet."
                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                    }
                }
            }
        }

        function applyFilter(filterName){
            var roomCount = listModel.rowCount();
            for (var i = 0; i < roomCount; i++){
                var roomName = listModel.roomAt(i).displayName;
                if (roomName.toLowerCase().indexOf(filterName.toLowerCase()) !== -1) {
                    items.addGroups(i, 1, "filterGroup");
                } else {items.removeGroups(i, 1, "filterGroup");}
            }
        }
    }

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
                id: searchField
                width: parent.width
                height: 36
                leftPadding: mini ? 4 : 16
                topPadding: 0
                bottomPadding: 0
                anchors.verticalCenter: parent.verticalCenter

                background: Item {
                    Row {
                        anchors.fill: parent

                        MaterialIcon {
                            icon: "\ue8b6"
                            color: "white"

                            width: mini ? parent.width : parent.height
                            height: parent.height
                        }

                        Text {
                            height: parent.height
                            visible: !mini
                            text: "Search"
                            color: "white"
                            font.pointSize: 12
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Rectangle {
                        width: searchField.activeFocus || searchField.text ? parent.width : 0
                        height: parent.height
                        color: "white"

                        Behavior on width {
                            PropertyAnimation { easing.type: Easing.InOutCubic; duration: 200 }
                        }
                    }
                }

                onTextChanged: {
                    delegateModel.applyFilter(text);
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
                    color: Material.theme == Material.Light ? "#eaeaea" : "#242424"
                }

                Label {
                    z: 10
                    text: mini ? "Empty" : "Here? No, not here."
                    anchors.centerIn: parent
                    visible: listView.count === 0
                }
            }

            ListView {
                id: listView
                width: parent.width
                height: parent.height

                highlight: Rectangle {
                    color: Material.accent
                    opacity: 0.2
                }
                highlightMoveDuration: 250

                currentIndex: -1

                ScrollBar.vertical: ScrollBar { id: scrollBar }

                model: delegateModel
            }
        }
    }
}
