import QtQuick 2.10
import QtQuick.Controls 2.3

Rectangle {
    id: root

    property Connection currentConnection: null
    property var currentRoom: null

    function setRoom(room) {
        currentRoom = room
        messageModel.changeRoom(room)
    }

    function setConnection(conn) {
        currentConnection = conn
        messageModel.setConnection(conn)
    }

    function sendLine(text) {
        if(!currentRoom || !currentConnection) return
        currentConnection.postMessage(currentRoom, "m.text", text)
    }

    ListView {
        id: chatView
        anchors.fill: parent
        flickableDirection: Flickable.VerticalFlick
        verticalLayoutDirection: ListView.BottomToTop
        model: MessageEventModel { id: messageModel }

        delegate: Row {
            id: message
            width: parent.width
            spacing: 8

            Label {
                id: timelabel
                text: time.toLocaleTimeString("hh:mm:ss")
                color: "grey"
            }
            Label {
                width: 64
                elide: Text.ElideRight
                text: eventType == "message" ? author : "***"
                color: eventType == "message" ? "grey" : "lightgrey"
                horizontalAlignment: Text.AlignRight
            }
            Label {
                text: content
                wrapMode: Text.Wrap
                width: parent.width - (x - parent.x) - spacing
                color: eventType == "message" ? "black" : "lightgrey"
            }
        }

        section {
            property: "date"
            labelPositioning: ViewSection.CurrentLabelAtStart
            delegate: Rectangle {
                width: parent.width
                height: childrenRect.height
                Label {
                    width: parent.width
                    text: section.toLocaleString(Qt.locale())
                    color: "grey"
                    horizontalAlignment: Text.AlignRight
                }
            }
        }

        onAtYBeginningChanged: {
            if(currentRoom && atYBeginning) currentRoom.getPreviousContent()
        }
    }
}
