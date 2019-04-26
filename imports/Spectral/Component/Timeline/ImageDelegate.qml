import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.0
import Qt.labs.platform 1.0 as Platform

import Spectral 0.1
import Spectral.Setting 0.1

import Spectral.Component 2.0
import Spectral.Dialog 2.0
import Spectral.Effect 2.0
import Spectral.Font 0.1

ColumnLayout {
    readonly property bool avatarVisible: !sentByMe && (aboveAuthor !== author || aboveSection !== section || aboveEventType === "state" || aboveEventType === "emote" || aboveEventType === "other")
    readonly property bool sentByMe: author === currentRoom.localUser

    property bool openOnFinished: false
    readonly property bool downloaded: progressInfo && progressInfo.completed

    id: root

    spacing: 0

    onDownloadedChanged: if (downloaded && openOnFinished) openSavedFile()

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
            source: author.avatarMediaId

            Component {
                id: userDetailDialog

                UserDetailDialog {}
            }

            RippleEffect {
                anchors.fill: parent

                circular: true

                onClicked: userDetailDialog.createObject(ApplicationWindow.overlay, {"room": currentRoom, "user": author}).open()
            }
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

        BusyIndicator {
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64

            visible: img.status == Image.Loading
        }

        Image {
            Layout.maximumWidth: messageListView.width - (!sentByMe ? 32 + messageRow.spacing : 0) - 48

            id: img

            source: "image://mxc/" +
                    (content.info && content.info.thumbnail_info ?
                         content.thumbnailMediaId : content.mediaId)

            sourceSize.width: 720
            sourceSize.height: 720

            fillMode: Image.PreserveAspectCrop

            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: Rectangle {
                    width: img.width
                    height: img.height
                    radius: 24
                }
            }

            Component {
                id: fullScreenImage

                FullScreenImage {
                    imageUrl: "image://mxc/" + content.mediaId
                    sourceWidth: content.info.w
                    sourceHeight: content.info.h
                }
            }

            Rectangle {
                anchors.fill: parent

                color: "transparent"
                radius: 24
                antialiasing: true

                border.width: 4
                border.color: MPalette.banner
            }

            RippleEffect {
                anchors.fill: parent

                id: messageMouseArea

                onPrimaryClicked: {
                    var window = fullScreenImage.createObject()
                    window.show()
                }

                onSecondaryClicked: messageContextMenu.popup()

                Component {
                    id: messageSourceDialog

                    MessageSourceDialog {}
                }

                Menu {
                    id: messageContextMenu

                    MenuItem {
                        text: "View Source"

                        onTriggered: messageSourceDialog.createObject(ApplicationWindow.overlay, {"sourceText": toolTip}).open()
                    }

                    MenuItem {
                        text: "Open Externally"

                        onTriggered: downloadAndOpen()
                    }

                    MenuItem {
                        text: "Save As"

                        onTriggered: saveFileAs()
                    }

                    MenuItem {
                        text: "Reply"

                        onTriggered: {
                            roomPanelInput.replyUser = author
                            roomPanelInput.replyEventID = eventId
                            roomPanelInput.replyContent = message
                            roomPanelInput.isReply = true
                            roomPanelInput.focus()
                        }
                    }

                    MenuItem {
                        text: "Redact"

                        onTriggered: currentRoom.redactEvent(eventId)
                    }
                }
            }
        }
    }

    function saveFileAs() { currentRoom.saveFileAs(eventId) }

    function downloadAndOpen()
    {
        if (downloaded) openSavedFile()
        else
        {
            openOnFinished = true
            currentRoom.downloadFile(eventId, Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_") + (message || ".tmp"))
        }
    }

    function openSavedFile()
    {
        if (Qt.openUrlExternally(progressInfo.localPath)) return;
        if (Qt.openUrlExternally(progressInfo.localDir)) return;
    }
}
