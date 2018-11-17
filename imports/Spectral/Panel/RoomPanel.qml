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

    Image {
        anchors.fill: parent

        visible: currentRoom

        source: MSettings.darkTheme ? "qrc:/assets/img/roompanel-dark.svg" : "qrc:/assets/img/roompanel.svg"
        fillMode: Image.PreserveAspectCrop
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

            paintable: currentRoom ? currentRoom.paintable : null
            topic: currentRoom ? (currentRoom.topic).replace(/(\r\n\t|\n|\r\t)/gm,"") : ""
            atTop: messageListView.atYBeginning

            onClicked: roomDrawer.open()
        }

        AutoListView {
            Layout.fillWidth: true
            Layout.maximumWidth: 960
            Layout.fillHeight: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.alignment: Qt.AlignHCenter

            id: messageListView

            displayMarginBeginning: 100
            displayMarginEnd: 100
            verticalLayoutDirection: ListView.BottomToTop
            spacing: 4

            boundsBehavior: Flickable.DragOverBounds

            model: SortFilterProxyModel {
                id: sortedMessageEventModel

                sourceModel: messageEventModel

                filters: ExpressionFilter {
                    expression: marks !== 0x08 && marks !== 0x10 && eventType !== "other"
                }

                onModelReset: {
                    if (currentRoom) {
                        var lastScrollPosition = sortedMessageEventModel.mapFromSource(currentRoom.savedTopVisibleIndex())
                        messageListView.currentIndex = lastScrollPosition
                        if (messageListView.contentY < messageListView.originY + 10 || currentRoom.timelineSize < 20)
                            currentRoom.getPreviousContent(100)
                    }
                }
            }

            property int largestVisibleIndex: count > 0 ? indexAt(contentX, contentY + height - 1) : -1

            onContentYChanged: {
                if(currentRoom && contentY  - 5000 < originY)
                    currentRoom.getPreviousContent(50);
            }

            onMovementEnded: currentRoom.saveViewport(sortedMessageEventModel.mapToSource(indexAt(contentX, contentY)), sortedMessageEventModel.mapToSource(largestVisibleIndex))

            displaced: Transition {
                NumberAnimation {
                    property: "y"; duration: 200
                    easing.type: Easing.OutQuad
                }
            }

            delegate: ColumnLayout {
                width: parent.width

                id: delegateColumn

                spacing: 4

                SectionDelegate {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.margins: 4

                    visible: section !== aboveSection || Math.abs(time - aboveTime) > 600000
                }

                MessageDelegate {
                    visible: eventType === "notice" || eventType === "message"
                             || eventType === "image" || eventType === "video"
                             || eventType === "audio" || eventType === "file"
                }

                StateDelegate {
                    Layout.maximumWidth: parent.width
                    Layout.alignment: Qt.AlignHCenter

                    visible: eventType === "emote" || eventType === "state"
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter

                    visible: eventType === "other"

                    text: display
                    color: "grey"
                    font.italic: true
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter

                    visible: readMarker === true

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 2

                        color: Material.accent
                    }

                    Label {
                        text: "And Now"
                        color: Material.accent
                        verticalAlignment: Text.AlignVCenter
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 2

                        color: Material.accent
                    }
                }
            }

            RoundButton {
                width: 64
                height: 64
                anchors.right: parent.right
                anchors.top: parent.top

                id: goBottomFab

                visible: currentRoom && currentRoom.hasUnreadMessages

                contentItem: MaterialIcon {
                    anchors.fill: parent

                    icon: "\ue316"
                    color: "white"
                }

                Material.background: Material.accent

                onClicked: goToEvent(currentRoom.readMarkerEventId)
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

                onClicked: messageListView.positionViewAtBeginning()
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

                                source: modelData.paintable
                                hint: modelData.displayName
                            }

                            Label {
                                Layout.fillWidth: true

                                text: modelData.displayName
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {}
                }
            }
        }

        RoomPanelInput {
            Layout.fillWidth: true
            Layout.margins: 16
            Layout.maximumWidth: 960
            Layout.alignment: Qt.AlignHCenter

            id: roomPanelInput
        }
    }

    function goToEvent(eventID) {
        var index = messageEventModel.eventIDToIndex(eventID)
        if (index === -1) return
        messageListView.currentIndex = -1
        messageListView.currentIndex = sortedMessageEventModel.mapFromSource(index)
    }

    function saveReadMarker(room) {
        var readMarker = sortedMessageEventModel.get(messageListView.largestVisibleIndex).eventId
        if (!readMarker) return
        room.readMarkerEventId = readMarker
    }
}
