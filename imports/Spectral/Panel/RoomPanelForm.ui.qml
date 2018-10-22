import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

import Spectral.Component 2.0
import Spectral.Component.Emoji 2.0
import Spectral.Component.Timeline 2.0
import Spectral.Menu 2.0
import Spectral.Effect 2.0

import Spectral 0.1
import Spectral.Setting 0.1
import SortFilterProxyModel 0.2

import "qrc:/js/md.js" as Markdown
import "qrc:/js/util.js" as Util

Item {
    property var currentRoom: null

    property alias roomHeader: roomHeader
    property alias messageListView: messageListView
    property alias goTopFab: goTopFab
    property alias messageEventModel: messageEventModel
    property alias sortedMessageEventModel: sortedMessageEventModel
    property alias roomDrawer: roomDrawer

    id: root

    MessageEventModel {
        id: messageEventModel
        room: currentRoom
    }

    RoomDrawer {
        width: Math.min(root.width * 0.7, 480)
        height: root.height

        id: roomDrawer

        room: currentRoom
    }

    Label {
        anchors.centerIn: parent
        visible: !currentRoom
        text: "Please choose a room."
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        visible: currentRoom

        RoomHeader {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            z: 10

            id: roomHeader
        }

        AutoListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16

            id: messageListView

            displayMarginBeginning: 40
            displayMarginEnd: 40
            verticalLayoutDirection: ListView.BottomToTop
            spacing: 8

            boundsBehavior: Flickable.DragOverBounds

            model: SortFilterProxyModel {
                id: sortedMessageEventModel

                sourceModel: messageEventModel

                filters: ExpressionFilter {
                    expression: marks !== 0x08 && marks !== 0x10
                }
            }

            delegate: ColumnLayout {
                width: parent.width

                id: delegateColumn

                spacing: 8

                Label {
                    Layout.alignment: Qt.AlignHCenter

                    visible: section !== aboveSection

                    text: section
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 8
                    rightPadding: 8
                    topPadding: 4
                    bottomPadding: 4

                    background: Rectangle {
                        color: MSettings.darkTheme ? "#484848" : "grey"
                    }
                }

                MessageDelegate {
                    visible: eventType === "notice" || eventType === "message"
                             || eventType === "image" || eventType === "video"
                             || eventType === "audio" || eventType === "file"
                }

                StateDelegate {
                    Layout.maximumWidth: messageListView.width * 0.8

                    visible: eventType === "emote" || eventType === "state"
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter

                    visible: eventType === "other"

                    text: display
                    color: "grey"
                    font.italic: true
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter

                    visible: readMarker === true && index !== 0

                    text: "And Now"
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 8
                    rightPadding: 8
                    topPadding: 4
                    bottomPadding: 4

                    background: Rectangle {
                        color: MSettings.darkTheme ? "#484848" : "grey"
                    }
                }
            }

            RoundButton {
                width: 64
                height: 64
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                id: goTopFab

                visible: !messageListView.atYEnd

                contentItem: MaterialIcon {
                    anchors.fill: parent

                    icon: "\ue313"
                    color: "white"
                }

                Material.background: Material.accent
            }

            MessageContextMenu {
                id: messageContextMenu
            }

            Popup {
                property string sourceText

                x: (window.width - width) / 2
                y: (window.height - height) / 2
                width: 480

                id: sourceDialog

                parent: ApplicationWindow.overlay

                modal: true

                padding: 16

                closePolicy: Dialog.CloseOnEscape | Dialog.CloseOnPressOutside

                contentItem: ScrollView {
                    TextArea {
                        readOnly: true
                        selectByMouse: true

                        text: sourceDialog.sourceText
                    }
                }
            }

            Popup {
                property alias listModel: readMarkerListView.model

                x: (window.width - width) / 2
                y: (window.height - height) / 2
                width: 320

                id: readMarkerDialog

                parent: ApplicationWindow.overlay

                modal: true
                padding: 16

                closePolicy: Dialog.CloseOnEscape | Dialog.CloseOnPressOutside

                contentItem: AutoListView {
                    implicitHeight: Math.min(window.height - 64,
                                             readMarkerListView.contentHeight)

                    id: readMarkerListView

                    clip: true
                    boundsBehavior: Flickable.DragOverBounds

                    delegate: ItemDelegate {
                        width: parent.width
                        height: 48

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 12

                            ImageItem {
                                Layout.preferredWidth: height
                                Layout.fillHeight: true

                                image: modelData.avatar
                                hint: modelData.displayName
                            }

                            Label {
                                Layout.fillWidth: true

                                text: modelData.displayName
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            Layout.leftMargin: 16
            Layout.rightMargin: 16

            color: Material.background

            RoomPanelInput {
                anchors.verticalCenter: parent.top

                id: roomPanelInput

                width: parent.width
                height: 48
            }
        }
    }
}


/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
