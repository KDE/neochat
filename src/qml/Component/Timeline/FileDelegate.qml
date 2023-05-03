// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

TimelineContainer {
    id: fileDelegate

    onOpenContextMenu: openFileContext(model, fileDelegate)

    readonly property bool downloaded: progressInfo && progressInfo.completed
    property bool autoOpenFile: false

    onDownloadedChanged: if (autoOpenFile) {
        openSavedFile();
    }

    function saveFileAs() {
        const dialog = fileDialog.createObject(QQC2.ApplicationWindow.overlay)
        dialog.open()
        dialog.currentFile = dialog.folder + "/" + currentRoom.fileNameToDownload(eventId)
    }

    function openSavedFile() {
        UrlHelper.openUrl(progressInfo.localPath);
    }

    innerObject: RowLayout {
        Layout.maximumWidth: Math.min(fileDelegate.contentMaxWidth, implicitWidth)

        spacing: Kirigami.Units.largeSpacing

        states: [
            State {
                name: "downloadedInstant"
                when: progressInfo.completed && autoOpenFile

                PropertyChanges {
                    target: openButton
                    icon.name: "document-open"
                    onClicked: openSavedFile()
                }

                PropertyChanges {
                    target: downloadButton
                    icon.name: "download"
                    QQC2.ToolTip.text: i18nc("tooltip for a button on a message; offers ability to download its file", "Download")
                    onClicked: saveFileAs()
                }
            },
            State {
                name: "downloaded"
                when: progressInfo.completed && !autoOpenFile

                PropertyChanges {
                    target: openButton
                    visible: false
                }

                PropertyChanges {
                    target: downloadButton
                    icon.name: "document-open"
                    QQC2.ToolTip.text: i18nc("tooltip for a button on a message; offers ability to open its downloaded file with an appropriate application", "Open File")
                    onClicked: openSavedFile()
                }
            },
            State {
                name: "downloading"
                when: progressInfo.active

                PropertyChanges {
                    target: openButton
                    visible: false
                }

                PropertyChanges {
                    target: sizeLabel
                    text: i18nc("file download progress", "%1 / %2", Controller.formatByteSize(progressInfo.progress), Controller.formatByteSize(progressInfo.total))
                }
                PropertyChanges {
                    target: downloadButton
                    icon.name: "media-playback-stop"
                    QQC2.ToolTip.text: i18nc("tooltip for a button on a message; stops downloading the message's file", "Stop Download")
                    onClicked: currentRoom.cancelFileTransfer(eventId)
                }
            },
            State {
                name: "raw"
                when: true

                PropertyChanges {
                    target: downloadButton
                    onClicked: fileDelegate.saveFileAs()
                }
            }
        ]

        Kirigami.Icon {
            source: model.mediaInfo.mimeIcon
            fallback: "unknown"
        }

        ColumnLayout {
            spacing: 0
            QQC2.Label {
                Layout.fillWidth: true
                text: model.display
                wrapMode: Text.Wrap
                elide: Text.ElideRight
            }
            QQC2.Label {
                id: sizeLabel
                Layout.fillWidth: true
                text: Controller.formatByteSize(model.mediaInfo.size)
                opacity: 0.7
                elide: Text.ElideRight
                maximumLineCount: 1
            }
        }

        QQC2.Button {
            id: openButton
            icon.name: "document-open"
            onClicked: {
                autoOpenFile = true;
                currentRoom.downloadTempFile(eventId);
            }

            QQC2.ToolTip.text: i18nc("tooltip for a button on a message; offers ability to open its downloaded file with an appropriate application", "Open File")
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        QQC2.Button {
            id: downloadButton
            icon.name: "download"

            QQC2.ToolTip.text: i18nc("tooltip for a button on a message; offers ability to download its file", "Download")
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        Component {
            id: fileDialog

            FileDialog {
                fileMode: FileDialog.SaveFile
                folder: Config.lastSaveDirectory.length > 0 ? Config.lastSaveDirectory : StandardPaths.writableLocation(StandardPaths.DownloadLocation)
                onAccepted: {
                    Config.lastSaveDirectory = folder
                    Config.save()
                    if (autoOpenFile) {
                        UrlHelper.copyTo(progressInfo.localPath, file)
                    } else {
                        currentRoom.download(eventId, file);
                    }
                }
            }
        }
    }
}
