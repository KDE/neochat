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
    property alias listModel: sortedRoomListModel.sourceModel
    property int filter: 0
    property var enteredRoom: null

    property alias searchField: searchField
    property alias sortedRoomListModel: sortedRoomListModel

    color: MSettings.darkTheme ? "#323232" : "#f3f3f3"

    Label {
        text: MSettings.miniMode ? "Empty" : "Here? No, not here."
        anchors.centerIn: parent
        visible: listView.count === 0
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TextField {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            Layout.margins: 12

            id: searchField

            leftPadding: MSettings.miniMode ? 4 : 32
            topPadding: 0
            bottomPadding: 0
            placeholderText: "Search..."

            background: Rectangle {
                color: MSettings.darkTheme ? "#303030" : "#fafafa"
                layer.enabled: true
                layer.effect: ElevationEffect {
                    elevation: searchField.focus ? 2 : 1
                }
            }
        }

        SortFilterProxyModel {
            id: sortedRoomListModel

            sorters: [
                RoleSorter { roleName: "category" },
                RoleSorter {
                    roleName: "lastActiveTime"
                    sortOrder: Qt.DescendingOrder
                }
            ]
        }

        SortFilterProxyModel {
            id: roomListProxyModel

            sourceModel: sortedRoomListModel

            filters: [
                RegExpFilter {
                    roleName: "name"
                    pattern: searchField.text
                    caseSensitivity: Qt.CaseInsensitive
                },
                ExpressionFilter {
                    enabled: filter === 1
                    expression: unreadCount > 0
                },
                ExpressionFilter {
                    enabled: filter === 2
                    expression: category === 1 || category === 2 || category === 4
                },
                ExpressionFilter {
                    enabled: filter === 3
                    expression: category === 3 || category === 5
                }
            ]
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: listView

            spacing: 1
            clip: true

            model: roomListProxyModel

            currentIndex: -1

            highlightFollowsCurrentItem: true

            highlightMoveDuration: 200
            highlightResizeDuration: 0

            boundsBehavior: Flickable.DragOverBounds

            ScrollBar.vertical: ScrollBar {}

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
                leftPadding: MSettings.miniMode ? undefined : 16
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: MSettings.miniMode ? Text.AlignHCenter : undefined
            }

            RoomContextMenu { id: roomContextMenu }
        }
    }
}
