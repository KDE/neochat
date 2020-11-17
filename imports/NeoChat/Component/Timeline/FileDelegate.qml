/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.0
import Qt.labs.platform 1.0 as Platform
import org.kde.kirigami 2.13 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Setting 1.0

import NeoChat.Component 1.0
import NeoChat.Dialog 1.0
import NeoChat.Menu.Timeline 1.0
import NeoChat.Effect 1.0

RowLayout {
    property bool openOnFinished: false
    readonly property bool downloaded: progressInfo && progressInfo.completed

    id: root

    spacing: 4

    onDownloadedChanged: if (downloaded && openOnFinished) openSavedFile()

    z: -5

    Control {
        contentItem: RowLayout {
            ToolButton {
                icon.name: progressInfo.completed ? "document-open" : "document-save"
                onClicked: progressInfo.completed ? openSavedFile() : saveFileAs()
            }

            ColumnLayout {
                Kirigami.Heading {
                    Layout.fillWidth: true
                    level: 4
                    text: display
                    wrapMode: Label.Wrap
                }

                Label {
                    Layout.fillWidth: true
                    text: !progressInfo.completed && progressInfo.active ? (humanSize(progressInfo.progress) + "/" + humanSize(progressInfo.total)) : humanSize(content.info ? content.info.size : 0)
                    color: Kirigami.Theme.disabledTextColor
                    wrapMode: Label.Wrap
                }
            }
        }

        background: Item {
            MouseArea {
                id: messageMouseArea
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: {
                    var contextMenu = fileDelegateContextMenu.createObject(root)
                    contextMenu.viewSource.connect(function() {
                        messageSourceSheet.createObject(ApplicationWindow.overlay, {"sourceText": toolTip}).open()
                    })
                    contextMenu.downloadAndOpen.connect(downloadAndOpen)
                    contextMenu.saveFileAs.connect(saveFileAs)
                    contextMenu.reply.connect(function() {
                        roomPanelInput.replyModel = Object.assign({}, model)
                        roomPanelInput.isReply = true
                        roomPanelInput.focus()
                    })
                    contextMenu.redact.connect(function() {
                        currentRoom.redactEvent(eventId)
                    })
                    contextMenu.popup()
                }

                Component {
                    id: messageSourceSheet

                    MessageSourceSheet {}
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
            return i18nc("Unknown attachment size", "Unknown")
        if (bytes < 4000)
            return i18np("%1 byte", "%1 bytes", bytes)
        bytes = Math.round(bytes / 100) / 10
        if (bytes < 2000)
            return i18nc("KB as in kilobytes", "%1 KB", bytes)
        bytes = Math.round(bytes / 100) / 10
        if (bytes < 2000)
            return i18nc("MB as in megabytes", "%1 MB", bytes)
        return i18nc("GB as in gigabytes", "%1 GB", Math.round(bytes / 100) / 10)
    }
}
