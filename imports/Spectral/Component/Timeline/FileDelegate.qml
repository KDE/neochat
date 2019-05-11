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
import Spectral.Menu.Timeline 2.0
import Spectral.Font 0.1
import Spectral.Effect 2.0

RowLayout {
    readonly property bool avatarVisible: !sentByMe && showAuthor
    readonly property bool sentByMe: author === currentRoom.localUser

    property bool openOnFinished: false
    readonly property bool downloaded: progressInfo && progressInfo.completed

    id: root

    spacing: 4

    onDownloadedChanged: if (downloaded && openOnFinished) openSavedFile()

    z: -5

    Avatar {
        Layout.preferredWidth: 36
        Layout.preferredHeight: 36
        Layout.alignment: Qt.AlignBottom

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
        Layout.preferredWidth: 36
        Layout.preferredHeight: 36

        visible: !(sentByMe || avatarVisible)
    }

    Control {
        Layout.maximumWidth: messageListView.width - (!sentByMe ? 36 + root.spacing : 0) - 48

        padding: 12

        contentItem: RowLayout {
            ToolButton {
                contentItem: MaterialIcon {
                    icon: progressInfo.completed ? "\ue5ca" : "\ue2c4"
                }

                onClicked: progressInfo.completed ? openSavedFile() : saveFileAs()
            }

            ColumnLayout {
                Label {
                    Layout.fillWidth: true

                    text: display
                    wrapMode: Label.Wrap
                    font.pixelSize: 18
                    font.weight: Font.Medium
                    font.capitalization: Font.AllUppercase
                }

                Label {
                    Layout.fillWidth: true

                    text: progressInfo.active ? (humanSize(progressInfo.progress) + "/" + humanSize(progressInfo.total)) : humanSize(content.info ? content.info.size : 0)
                    color: MPalette.lighter
                    wrapMode: Label.Wrap
                }
            }
        }

        background: Rectangle {
            color: MPalette.background
            radius: 18

            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left

                width: parent.width / 2
                height: parent.height / 2

                visible: !sentByMe && (bubbleShape == 3 || bubbleShape == 2)

                color: sentByMe ? MPalette.background : eventType === "notice" ? MPalette.primary : MPalette.accent
                radius: 2
            }

            Rectangle {
                anchors.top: parent.top
                anchors.right: parent.right

                width: parent.width / 2
                height: parent.height / 2

                visible: sentByMe && (bubbleShape == 3 || bubbleShape == 2)

                color: sentByMe ? MPalette.background : eventType === "notice" ? MPalette.primary : MPalette.accent
                radius: 2
            }

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left

                width: parent.width / 2
                height: parent.height / 2

                visible: !sentByMe && (bubbleShape == 1 || bubbleShape == 2)

                color: sentByMe ? MPalette.background : eventType === "notice" ? MPalette.primary : MPalette.accent
                radius: 2
            }

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.right: parent.right

                width: parent.width / 2
                height: parent.height / 2

                visible: sentByMe && (bubbleShape == 1 || bubbleShape == 2)

                color: sentByMe ? MPalette.background : eventType === "notice" ? MPalette.primary : MPalette.accent
                radius: 2
            }

            AutoMouseArea {
                anchors.fill: parent

                id: messageMouseArea

                onSecondaryClicked: {
                    var contextMenu = fileDelegateContextMenu.createObject(ApplicationWindow.overlay)
                    contextMenu.viewSource.connect(function() {
                        messageSourceDialog.createObject(ApplicationWindow.overlay, {"sourceText": toolTip}).open()
                    })
                    contextMenu.downloadAndOpen.connect(downloadAndOpen)
                    contextMenu.saveFileAs.connect(saveFileAs)
                    contextMenu.reply.connect(function() {
                        roomPanelInput.replyUser = author
                        roomPanelInput.replyEventID = eventId
                        roomPanelInput.replyContent = message
                        roomPanelInput.isReply = true
                        roomPanelInput.focus()
                    })
                    contextMenu.redact.connect(function() {
                        currentRoom.redactEvent(eventId)
                    })
                    contextMenu.popup()
                }

                Component {
                    id: messageSourceDialog

                    MessageSourceDialog {}
                }

                Component {
                    id: openFolderDialog

                    OpenFolderDialog {}
                }

                Component {
                    id: fileDelegateContextMenu

                    FileDelegateContextMenu {}
                }
            }
        }
    }

    function saveFileAs() {
        var folderDialog = openFolderDialog.createObject(ApplicationWindow.overlay)

        folderDialog.chosen.connect(function(path) {
            if (!path) return

            currentRoom.downloadFile(eventId, path + "/" + currentRoom.fileNameToDownload(eventId))
        })

        folderDialog.open()
    }

    function downloadAndOpen()
    {
        if (downloaded) openSavedFile()
        else
        {
            openOnFinished = true
            currentRoom.downloadFile(eventId, Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId))
        }
    }

    function openSavedFile()
    {
        if (Qt.openUrlExternally(progressInfo.localPath)) return;
        if (Qt.openUrlExternally(progressInfo.localDir)) return;
    }

    function humanSize(bytes)
    {
        if (!bytes)
            return qsTr("Unknown", "Unknown attachment size")
        if (bytes < 4000)
            return qsTr("%1 bytes").arg(bytes)
        bytes = Math.round(bytes / 100) / 10
        if (bytes < 2000)
            return qsTr("%1 KB").arg(bytes)
        bytes = Math.round(bytes / 100) / 10
        if (bytes < 2000)
            return qsTr("%1 MB").arg(bytes)
        return qsTr("%1 GB").arg(Math.round(bytes / 100) / 10)
    }
}
