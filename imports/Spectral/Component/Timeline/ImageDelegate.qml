import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.4
import QtGraphicalEffects 1.0

import Spectral 0.1
import Spectral.Setting 0.1

import Spectral.Component 2.0
import Spectral.Font 0.1

ColumnLayout {
    readonly property bool avatarVisible: !sentByMe && (aboveAuthor !== author || aboveSection !== section || aboveEventType === "state" || aboveEventType === "emote" || aboveEventType === "other")
    readonly property bool sentByMe: author === currentRoom.localUser

    signal saveFileAs()
    signal openExternally()

    id: root

    spacing: 0

    Label {
        Layout.leftMargin: 48

        text: author.displayName

        visible: avatarVisible

        font.pixelSize: 13
        verticalAlignment: Text.AlignVCenter
    }

    RowLayout {
        Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft

        z: -5

        id: messageRow

        spacing: 4

        Avatar {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignTop

            visible: avatarVisible
            hint: author.displayName
            source: author.avatarUrl
        }

        Label {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignTop

            visible: !(sentByMe || avatarVisible)

            text: Qt.formatDateTime(time, "hh:mm")
            color: "#5B7480"

            font.pixelSize: 10
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter
        }

        Image {
            Layout.maximumWidth: messageListView.width - (!sentByMe ? 32 + messageRow.spacing : 0) - 48

            id: img

            source: "image://mxc/" + (content.thumbnail_url ? content.thumbnail_url : content.url)
            sourceSize.width: Math.min(256, messageListView.width)

            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: Rectangle {
                        width: img.width
                        height: img.height
                        radius: 24
                    }
            }

            AutoMouseArea {
                anchors.fill: parent

                id: messageMouseArea

                onSecondaryClicked: {
                    messageContextMenu.root = root
                    messageContextMenu.model = model
                    messageContextMenu.selectedText = ""
                    messageContextMenu.popup()
                }
            }
        }
    }
}
