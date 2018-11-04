import QtQuick 2.9

RoomPanelForm {
    roomHeader.paintable: currentRoom ? currentRoom.paintable : null
    roomHeader.topic: currentRoom ? (currentRoom.topic).replace(/(\r\n\t|\n|\r\t)/gm,"") : ""
    roomHeader.onClicked: roomDrawer.open()

    sortedMessageEventModel.onModelReset: {
        if (currentRoom) {
            var lastScrollPosition = sortedMessageEventModel.mapFromSource(currentRoom.savedTopVisibleIndex())
            messageListView.currentIndex = lastScrollPosition
            if (messageListView.contentY < messageListView.originY + 10 || currentRoom.timelineSize < 20)
                currentRoom.getPreviousContent(100)
        }
    }

    messageListView {
        property int largestVisibleIndex: messageListView.count > 0 ? messageListView.indexAt(messageListView.contentX, messageListView.contentY + messageListView.height - 1) : -1

        onContentYChanged: {
            if(currentRoom && messageListView.contentY  - 5000 < messageListView.originY)
                currentRoom.getPreviousContent(50);
        }

        onMovementEnded: currentRoom.saveViewport(sortedMessageEventModel.mapToSource(messageListView.indexAt(messageListView.contentX, messageListView.contentY)), sortedMessageEventModel.mapToSource(largestVisibleIndex))

        displaced: Transition {
            NumberAnimation {
                property: "y"; duration: 200
                easing.type: Easing.OutQuad
            }
        }
    }

    goBottomFab.onClicked: goToEvent(currentRoom.readMarkerEventId)
    goTopFab.onClicked: messageListView.positionViewAtBeginning()

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
