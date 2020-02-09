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
            wrapMode: Label.Wrap
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter

            text: "To start chatting, select a room from the room list."
            wrapMode: Label.Wrap
        }
    }

    Rectangle {
        anchors.fill: parent

        visible: currentRoom
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
                readonly property int largestVisibleIndex: count > 0 ? indexAt(contentX + (width / 2), contentY + height - 1) : -1
                readonly property bool noNeedMoreContent: !currentRoom || currentRoom.eventsHistoryJob || currentRoom.allHistoryLoaded

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
                        movingTimer.stop()
                        if (currentRoom) {
                            movingTimer.restart()

                            //                            var lastScrollPosition = sortedMessageEventModel.mapFromSource(currentRoom.savedTopVisibleIndex())
                            //                            if (lastScrollPosition === 0) {
                            //                                messageListView.positionViewAtBeginning()
                            //                            } else {
                            //                                messageListView.currentIndex = lastScrollPosition
                            //                            }

                            if (messageListView.contentY < messageListView.originY + 10 || currentRoom.timelineSize < 20)
                                currentRoom.getPreviousContent(50)
                        }
                    }
                }

                onContentYChanged: {
                    if(!noNeedMoreContent && contentY  - 5000 < originY)
                        currentRoom.getPreviousContent(20);
                }

                populate: Transition {
                    NumberAnimation {
                        property: "opacity"; from: 0; to: 1
                        duration: 200
                    }
                }

                add: Transition {
                    NumberAnimation {
                        property: "opacity"; from: 0; to: 1
                        duration: 200
                    }
                }

                move: Transition {
                    NumberAnimation {
                        property: "y"; duration: 200
                    }
                    NumberAnimation {
                        property: "opacity"; to: 1
                    }
                }

                displaced: Transition {
                    NumberAnimation {
                        property: "y"; duration: 200
                        easing.type: Easing.OutQuad
                    }
                    NumberAnimation {
                        property: "opacity"; to: 1
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

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 2

                                visible: readMarker

                                color: MPalette.primary
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

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 2

                                visible: readMarker

                                color: MPalette.primary
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

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 2

                                visible: readMarker

                                color: MPalette.primary
                            }
                        }
                    }

                    DelegateChoice {
                        roleValue: "audio"
                        delegate: ColumnLayout {
                            width: messageListView.width

                            SectionDelegate {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.maximumWidth: parent.width

                                visible: showSection
                            }

                            AudioDelegate {
                                Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 2

                                visible: readMarker

                                color: MPalette.primary
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

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 2

                                visible: readMarker

                                color: MPalette.primary
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

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 2

                                visible: readMarker

                                color: MPalette.primary
                            }
                        }
                    }

                    DelegateChoice {
                        roleValue: "other"
                        delegate: Item {}
                    }
                }

                Control {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 16

                    padding: 8

                    id: goReadMarkerFab

                    visible: currentRoom && currentRoom.hasUnreadMessages

                    contentItem: MaterialIcon {
                        icon: "\ue316"
                        font.pixelSize: 28
                    }

                    background: Rectangle {
                        color: MPalette.background
                        radius: height / 2

                        layer.enabled: true
                        layer.effect: ElevationEffect {
                            elevation: 2
                        }

                        RippleEffect {
                            anchors.fill: parent

                            circular: true

                            onClicked: goToEvent(currentRoom.readMarkerEventId)
                        }
                    }
                }

                Control {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    padding: 8

                    id: goTopFab

                    visible: !messageListView.atYEnd

                    contentItem: MaterialIcon {
                        icon: "\ue313"
                        font.pixelSize: 28
                    }

                    background: Rectangle {
                        color: MPalette.background
                        radius: height / 2

                        layer.enabled: true
                        layer.effect: ElevationEffect {
                            elevation: 2
                        }

                        RippleEffect {
                            anchors.fill: parent

                            circular: true

                            onClicked: {
                                currentRoom.markAllMessagesAsRead()
                                messageListView.positionViewAtBeginning()
                            }
                        }
                    }
                }


                Control {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom

                    visible: currentRoom && currentRoom.usersTyping.length > 0
                    padding: 4

                    contentItem: RowLayout {
                        spacing: 0

                        RowLayout {
                            spacing: -8

                            Repeater {
                                model: currentRoom && currentRoom.usersTyping.length > 0 ? currentRoom.usersTyping : null

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
                                        color: modelData.color
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

                ScrollBar.vertical: ScrollBar {
                    id: scrollBar
                }
            }

            RoomPanelInput {
                Layout.fillWidth: true

                id: roomPanelInput
            }
        }
    }

    Timer {
        id: movingTimer

        interval: 10000
        repeat: true
        running: false

        onTriggered: saveReadMarker()
    }

    function goToEvent(eventID) {
        var index = messageEventModel.eventIDToIndex(eventID)
        if (index === -1) return
        messageListView.positionViewAtIndex(sortedMessageEventModel.mapFromSource(index), ListView.Contain)
    }

    function saveViewport() {
        currentRoom.saveViewport(sortedMessageEventModel.mapToSource(messageListView.indexAt(messageListView.contentX + (messageListView.width / 2), messageListView.contentY)), sortedMessageEventModel.mapToSource(messageListView.largestVisibleIndex))
    }

    function saveReadMarker() {
        var readMarker = sortedMessageEventModel.get(messageListView.largestVisibleIndex).eventId
        if (!readMarker) return
        currentRoom.readMarkerEventId = readMarker
    }
}
