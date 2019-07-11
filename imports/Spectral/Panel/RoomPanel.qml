import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import Qt.labs.qmlmodels 1.0
import Qt.labs.platform 1.0
import QtGraphicalEffects 1.0

import Spectral.Component 2.0
import Spectral.Component.Emoji 2.0
import Spectral.Component.Timeline 2.0
import Spectral.Dialog 2.0
import Spectral.Effect 2.0

import Spectral 0.1
import Spectral.Setting 0.1
import SortFilterProxyModel 0.2

Item {
    property var currentRoom: null

    id: root

    MessageEventModel {
        id: messageEventModel
        room: currentRoom
    }

    DropArea {
        anchors.fill: parent

        enabled: currentRoom

        onDropped: {
            if (!drop.hasUrls) return

            roomPanelInput.attach(drop.urls[0])
        }
    }

    ImageClipboard {
        id: imageClipboard
    }

    Popup {
        anchors.centerIn: parent

        id: attachDialog

        padding: 16

        contentItem: RowLayout {
            Control {
                Layout.preferredWidth: 160
                Layout.fillHeight: true

                padding: 16

                contentItem: ColumnLayout {
                    spacing: 16

                    MaterialIcon {
                        Layout.alignment: Qt.AlignHCenter

                        icon: "\ue2c8"
                        font.pixelSize: 64
                        color: MPalette.lighter
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter

                        text: "Choose local file"
                        color: MPalette.foreground
                    }
                }

                background: RippleEffect {
                    onClicked: {
                        attachDialog.close()

                        var fileDialog = openFileDialog.createObject(ApplicationWindow.overlay)

                        fileDialog.chosen.connect(function(path) {
                            if (!path) return

                            roomPanelInput.attach(path)
                        })

                        fileDialog.open()
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true

                color: MPalette.banner
            }

            Control {
                Layout.preferredWidth: 160
                Layout.fillHeight: true

                padding: 16

                contentItem: ColumnLayout {
                    spacing: 16

                    MaterialIcon {
                        Layout.alignment: Qt.AlignHCenter

                        icon: "\ue410"
                        font.pixelSize: 64
                        color: MPalette.lighter
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter

                        text: "Clipboard image"
                        color: MPalette.foreground
                    }
                }

                background: RippleEffect {
                    onClicked: {
                        var localPath = StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/screenshots/" + (new Date()).getTime() + ".png"
                        if (!imageClipboard.saveImage(localPath)) return
                        roomPanelInput.attach(localPath)
                        attachDialog.close()
                    }
                }
            }
        }
    }

    Component {
        id: openFileDialog

        OpenFileDialog {}
    }

    Column {
        anchors.centerIn: parent

        spacing: 16

        visible: !currentRoom

        Image {
            anchors.horizontalCenter: parent.horizontalCenter

            width: 240

            fillMode: Image.PreserveAspectFit

            source: "qrc:/assets/img/matrix.svg"
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter

            text: "Welcome to Matrix, a new era of instant messaging."
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter

            text: "To start chatting, select a room from the room list."
        }
    }

    Image {
        readonly property int sourceDim: (Math.ceil(Math.max(width, height) / 360) + 1) * 360

        anchors.fill: parent

        visible: currentRoom && currentRoom.backgroundMediaId

        sourceSize.width: sourceDim
        sourceSize.height: sourceDim

        fillMode: Image.PreserveAspectCrop

        source: currentRoom && currentRoom.backgroundMediaId ? "image://mxc/" + currentRoom.backgroundMediaId : ""
    }

    Rectangle {
        anchors.fill: parent

        visible: currentRoom && !currentRoom.backgroundMediaId
        color: MSettings.darkTheme ? "#242424" : "#EBEFF2"
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

            onClicked: roomDrawer.visible ? roomDrawer.close() : roomDrawer.open()
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumWidth: 960
            Layout.alignment: Qt.AlignHCenter
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.bottomMargin: 16

            width: Math.min(parent.width - 32, 960)

            spacing: 16

            AutoListView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                id: messageListView

                spacing: 2

                displayMarginBeginning: 100
                displayMarginEnd: 100
                verticalLayoutDirection: ListView.BottomToTop
                highlightMoveDuration: 500

                boundsBehavior: Flickable.DragOverBounds

                model: SortFilterProxyModel {
                    id: sortedMessageEventModel

                    sourceModel: messageEventModel

                    filters: [
                        ExpressionFilter {
                            expression: marks !== 0x10 && eventType !== "other"
                        }
                    ]

                    onModelReset: {
                        if (currentRoom) {
                            var lastScrollPosition = sortedMessageEventModel.mapFromSource(currentRoom.savedTopVisibleIndex())
                            messageListView.currentIndex = lastScrollPosition
                            if (messageListView.contentY < messageListView.originY + 10 || currentRoom.timelineSize < 20)
                                currentRoom.getPreviousContent(50)
                        }
                    }
                }

                property int largestVisibleIndex: count > 0 ? indexAt(contentX, contentY + height - 1) : -1

                onContentYChanged: {
                    if(currentRoom && contentY  - 5000 < originY)
                        currentRoom.getPreviousContent(20);
                }

                displaced: Transition {
                    NumberAnimation {
                        property: "y"; duration: 200
                        easing.type: Easing.OutQuad
                    }
                }

                delegate: DelegateChooser {
                    role: "eventType"

                    DelegateChoice {
                        roleValue: "state"
                        delegate: StateDelegate {
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    DelegateChoice {
                        roleValue: "emote"
                        delegate: StateDelegate {
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    DelegateChoice {
                        roleValue: "message"
                        delegate: ColumnLayout {
                            width: messageListView.width

                            SectionDelegate {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.maximumWidth: parent.width

                                visible: showSection
                            }

                            MessageDelegate {
                                Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft
                            }
                        }
                    }

                    DelegateChoice {
                        roleValue: "notice"
                        delegate: ColumnLayout {
                            width: messageListView.width

                            SectionDelegate {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.maximumWidth: parent.width

                                visible: showSection
                            }

                            MessageDelegate {
                                Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft
                            }
                        }
                    }

                    DelegateChoice {
                        roleValue: "image"
                        delegate: ColumnLayout {
                            width: messageListView.width

                            SectionDelegate {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.maximumWidth: parent.width

                                visible: showSection
                            }

                            ImageDelegate {
                                Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft
                            }
                        }
                    }

                    DelegateChoice {
                        roleValue: "video"
                        delegate: ColumnLayout {
                            width: messageListView.width

                            SectionDelegate {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.maximumWidth: parent.width

                                visible: showSection
                            }

                            VideoDelegate {
                                Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft
                            }
                        }
                    }

                    DelegateChoice {
                        roleValue: "file"
                        delegate: ColumnLayout {
                            width: messageListView.width

                            SectionDelegate {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.maximumWidth: parent.width

                                visible: showSection
                            }

                            FileDelegate {
                                Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft
                            }
                        }
                    }

                    DelegateChoice {
                        roleValue: "other"
                        delegate: Item {}
                    }
                }

                Button {
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter

                    visible: currentRoom && currentRoom.hasUnreadMessages

                    topPadding: 8
                    bottomPadding: 8
                    leftPadding: 24
                    rightPadding: 24

                    Material.foreground: MPalette.foreground
                    Material.background: MPalette.background

                    text: "Go to read marker"

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

                Control {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom

                    visible: currentRoom && currentRoom.hasUsersTyping
                    padding: 4

                    contentItem: RowLayout {
                        spacing: 0

                        RowLayout {
                            spacing: -8

                            Repeater {
                                model: currentRoom && currentRoom.hasUsersTyping ? currentRoom.usersTyping : null

                                delegate: Rectangle {
                                    Layout.preferredWidth: 28
                                    Layout.preferredHeight: 28

                                    color: "white"
                                    radius: 14

                                    Avatar {
                                        anchors.fill: parent
                                        anchors.margins: 2

                                        source: modelData.avatarMediaId
                                        hint: modelData.displayName
                                    }
                                }
                            }
                        }

                        Item {
                            Layout.preferredWidth: 28
                            Layout.preferredHeight: 28

                            BusyIndicator {
                                anchors.centerIn: parent

                                width: 32
                                height: 32
                            }
                        }
                    }

                    background: Rectangle {
                        color: MPalette.background
                        radius: height / 2

                        layer.enabled: true
                        layer.effect: ElevationEffect {
                            elevation: 1
                        }
                    }
                }

                Keys.onUpPressed: scrollBar.decrease()
                Keys.onDownPressed: scrollBar.increase()

                ScrollBar.vertical: ScrollBar { id: scrollBar }
            }

            RoomPanelInput {
                Layout.fillWidth: true

                id: roomPanelInput
            }
        }
    }

    function goToEvent(eventID) {
        var index = messageEventModel.eventIDToIndex(eventID)
        if (index === -1) return
        //        messageListView.currentIndex = sortedMessageEventModel.mapFromSource(index)
        messageListView.positionViewAtIndex(sortedMessageEventModel.mapFromSource(index), ListView.Contain)
    }

    function saveReadMarker(room) {
        var readMarker = sortedMessageEventModel.get(messageListView.largestVisibleIndex).eventId
        if (!readMarker) return
        room.readMarkerEventId = readMarker
        currentRoom.saveViewport(sortedMessageEventModel.mapToSource(messageListView.indexAt(messageListView.contentX, messageListView.contentY)), sortedMessageEventModel.mapToSource(messageListView.largestVisibleIndex))
    }
}
