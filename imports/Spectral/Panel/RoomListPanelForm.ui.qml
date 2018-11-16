import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.2
import QtQml.Models 2.3

import Spectral.Component 2.0
import Spectral.Menu 2.0
import Spectral.Effect 2.0

import Spectral 0.1
import Spectral.Setting 0.1
import SortFilterProxyModel 0.2

import "qrc:/js/util.js" as Util

Rectangle {
    property var listModel
    property int filter: 0
    property var enteredRoom: null

    property alias searchField: searchField
    property alias model: listView.model

    property bool miniMode: width == 64

    signal enterRoom(var room)
    signal leaveRoom(var room)

    color: MSettings.darkTheme ? "#323232" : "#f3f3f3"

    Label {
        text: miniMode ? "Empty" : "Here? No, not here."
        anchors.centerIn: parent
        visible: listView.count === 0
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            Layout.margins: 12

            color: MSettings.darkTheme ? "#303030" : "#fafafa"

            RowLayout {
                anchors.fill: parent

                spacing: 0

                MaterialIcon {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    visible: !miniMode && !searchField.text

                    icon: "\ue8b6"
                    color: "grey"
                }

                ItemDelegate {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    visible: !miniMode && searchField.text

                    contentItem: MaterialIcon {
                        icon: "\ue5cd"
                        color: "grey"
                    }

                    onClicked: searchField.text = ""
                }

                AutoTextField {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: searchField

                    topPadding: 0
                    bottomPadding: 0
                    placeholderText: "Search..."

                    background: Item {
                    }
                }
            }
        }

        AutoListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: listView

            spacing: 0
            clip: true

            boundsBehavior: Flickable.DragOverBounds

            ScrollBar.vertical: ScrollBar {
            }

            delegate: RoomListDelegate {
                width: parent.width
                height: 64
            }

            section.property: "display"
            section.criteria: ViewSection.FullString
            section.delegate: Label {
                width: parent.width
                height: 24

                text: section
                color: "grey"
                leftPadding: miniMode ? undefined : 16
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: miniMode ? Text.AlignHCenter : undefined
            }

            RoomContextMenu {
                id: roomContextMenu
            }
        }
    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
