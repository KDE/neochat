import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

Item {
    readonly property bool sentByMe: author === currentRoom.localUser

    anchors.right: messageRow.visible && sentByMe ? parent.right : undefined
    anchors.horizontalCenter: stateText.visible ? parent.horizontalCenter : undefined

    width: {
        if (messageRow.visible) return messageRow.width
        if (stateText.visible) return stateText.width
    }
    height: {
        if (messageRow.visible) return messageRow.height
        if (stateText.visible) return stateText.height
    }

    MouseArea {
        id: baseMouseArea
        anchors.fill: parent

        ToolTip.visible: pressed
        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
        ToolTip.text: time
    }

    Row {
        id: messageRow
        visible: eventType === "message" || eventType === "image" || eventType === "notice"

        spacing: 6

        ImageStatus {
            id: avatar

            width: height
            height: 40
            round: false
            visible: !sentByMe
            source: author.avatarUrl != "" ? "image://mxc/" + author.avatarUrl : null
            displayText: author.displayName

            MouseArea {
                anchors.fill: parent

                hoverEnabled: true
                ToolTip.visible: containsMouse
                ToolTip.text: author.displayName
            }
        }

        Rectangle {
            id: messageRect

            width: {
                if (eventType === "image") return messageImage.width + 24
                if (eventType === "message")
                    return Math.min(messageText.implicitWidth + 24, messageListView.width - (!sentByMe ? avatar.width + messageRow.spacing : 0))
                if (eventType === "notice")
                    return Math.min(noticeText.implicitWidth + 24, messageListView.width - (!sentByMe ? avatar.width + messageRow.spacing : 0))
            }
            height: {
                if (eventType === "image") return messageImage.height + 24
                if (eventType === "message") return messageText.implicitHeight + 24
                if (eventType === "notice") return noticeText.implicitHeight + 24
            }

            color: noticeText.visible ? "transparent" : sentByMe ? "lightgrey" : Material.accent
            border.color: Material.accent
            border.width: noticeText.visible ? 2 : 0

            Label {
                id: messageText
                visible: eventType === "message"
                text: display
                color: sentByMe ? "black" : "white"
                anchors.fill: parent
                anchors.margins: 12
                wrapMode: Label.Wrap
                textFormat: Text.RichText
            }

            Label {
                id: noticeText
                visible: eventType === "notice"
                text: display
                color: "black"
                anchors.fill: parent
                anchors.margins: 12
                wrapMode: Label.Wrap
                textFormat: Text.RichText
            }

            Image {
                id: messageImage
                anchors.centerIn: parent
                visible: eventType === "image"
                source: visible? "image://mxc/" + content.url : ""

                MouseArea {
                    anchors.fill: parent

                    hoverEnabled: true
                    propagateComposedEvents: true
                    ToolTip.visible: containsMouse
                    ToolTip.text: visible ? content.body : ""
                }
            }
        }
    }

    Label {
        id: stateText
        visible: eventType === "state" || eventType === "emote"
        width: Math.min(implicitWidth, messageListView.width)
        height: implicitHeight
        padding: 12
        text: author.displayName + " " + display
        color: eventType === "state" ? "black" : "white"
        wrapMode: Label.Wrap
        textFormat: Text.StyledText

        background: Rectangle {
            anchors.fill: parent
            color: eventType === "state" ? "lightgrey" : Material.accent
        }
    }
}
