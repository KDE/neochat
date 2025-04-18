// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-3.0-only

import QtCore as Core
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Dialogs as Dialogs
import Qt.labs.qmlmodels

import org.kde.coreaddons
import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component to show a file from a message.
 */
ColumnLayout {
    id: root

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The media info for the event.
     *
     * This should consist of the following:
     *  - source - The mxc URL for the media.
     *  - mimeType - The MIME type of the media (should be image/xxx for this delegate).
     *  - mimeIcon - The MIME icon name (should be image-xxx).
     *  - size - The file size in bytes.
     *  - width - The width in pixels of the audio media.
     *  - height - The height in pixels of the audio media.
     *  - tempInfo - mediaInfo (with the same properties as this except no tempInfo) for a temporary image while the file downloads.
     *  - filename - original filename of the media
     */
    required property var mediaInfo

    /**
     * @brief FileTransferInfo for any downloading files.
     */
    required property var fileTransferInfo

    /**
     * @brief Whether the media has been downloaded.
     */
    readonly property bool downloaded: root.fileTransferInfo && root.fileTransferInfo.completed
    onDownloadedChanged: {
        if (autoOpenFile) {
            openSavedFile();
        }
    }

    /**
     * @brief Whether the file should be automatically opened when downloaded.
     */
    property bool autoOpenFile: false

    function saveFileAs() {
        const dialog = fileDialog.createObject(QQC2.Overlay.overlay);
        dialog.selectedFile = Message.room.fileNameToDownload(root.eventId);
        dialog.open();
    }

    function openSavedFile() {
        UrlHelper.openUrl(root.fileTransferInfo.localPath);
    }

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth
    spacing: Kirigami.Units.largeSpacing

    RowLayout {
        spacing: Kirigami.Units.largeSpacing

        states: [
            State {
                name: "downloadedInstant"
                when: root.fileTransferInfo.completed && autoOpenFile

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
                when: root.fileTransferInfo.completed && !autoOpenFile

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
                when: root.fileTransferInfo.active

                PropertyChanges {
                    target: openButton
                    visible: false
                }

                PropertyChanges {
                    target: sizeLabel
                    text: i18nc("file download progress", "%1 / %2", Format.formatByteSize(root.fileTransferInfo.progress), Format.formatByteSize(root.fileTransferInfo.total))
                }
                PropertyChanges {
                    target: downloadButton
                    icon.name: "media-playback-stop"
                    QQC2.ToolTip.text: i18nc("tooltip for a button on a message; stops downloading the message's file", "Stop Download")
                    onClicked: Message.room.cancelFileTransfer(root.eventId)
                }
            }
        ]

        Kirigami.Icon {
            source: root.mediaInfo.mimeIcon
            fallback: "unknown"
        }

        ColumnLayout {
            spacing: 0
            QQC2.Label {
                Layout.fillWidth: true
                text: root.mediaInfo.filename
                wrapMode: Text.Wrap
                elide: Text.ElideRight
            }
            QQC2.Label {
                id: sizeLabel
                Layout.fillWidth: true
                text: Format.formatByteSize(root.mediaInfo.size)
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
                root.Message.room.downloadTempFile(root.eventId);
            }

            QQC2.ToolTip.text: i18nc("tooltip for a button on a message; offers ability to open its downloaded file with an appropriate application", "Open File")
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        QQC2.Button {
            id: downloadButton
            icon.name: "download"
            onClicked: root.saveFileAs()

            QQC2.ToolTip.text: i18nc("tooltip for a button on a message; offers ability to download its file", "Download")
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        Component {
            id: fileDialog

            Dialogs.FileDialog {
                fileMode: Dialogs.FileDialog.SaveFile
                currentFolder: NeoChatConfig.lastSaveDirectory.length > 0 ? NeoChatConfig.lastSaveDirectory : Core.StandardPaths.writableLocation(Core.StandardPaths.DownloadLocation)
                onAccepted: {
                    NeoChatConfig.lastSaveDirectory = currentFolder;
                    NeoChatConfig.save();
                    if (autoOpenFile) {
                        UrlHelper.copyTo(root.fileTransferInfo.localPath, selectedFile);
                    } else {
                        root.Message.room.download(root.eventId, selectedFile);
                    }
                }
            }
        }
    }
}
