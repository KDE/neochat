// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0
import NeoChat.Dialog 1.0
import NeoChat.Menu.Timeline 1.0

TimelineContainer {
    id: fileDelegate

    onReplyClicked: ListView.view.goToEvent(eventID)
    hoverComponent: hoverActions

    readonly property bool downloaded: progressInfo && progressInfo.completed

    function saveFileAs() {
        const dialog = fileDialog.createObject(QQC2.ApplicationWindow.overlay)
        dialog.open()
        dialog.currentFile = dialog.folder + "/" + currentRoom.fileNameToDownload(eventId)
    }

    function openSavedFile() {
        if (UrlHelper.openUrl(progressInfo.localPath)) return;
        if (UrlHelper.openUrl(progressInfo.localDir)) return;
    }

    innerObject: RowLayout {

        Layout.fillWidth: true
        Layout.maximumWidth: fileDelegate.contentMaxWidth
        Layout.margins: Kirigami.Units.largeSpacing

        spacing: Kirigami.Units.largeSpacing

        states: [
            State {
                name: "downloaded"
                when: progressInfo.completed

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
            id: ikon
            source: model.fileMimetypeIcon
            fallback: "unknown"
        }
        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true

            spacing: 0

            QQC2.Label {
                text: model.display
                wrapMode: Text.Wrap

                Layout.fillWidth: true
            }
            QQC2.Label {
                id: sizeLabel

                text: Controller.formatByteSize(content.info ? content.info.size : 0)
                opacity: 0.7

                Layout.fillWidth: true
            }
        }

        QQC2.Button {
            id: downloadButton
            icon.name: "download"

            QQC2.ToolTip.text: i18nc("tooltip for a button on a message; offers ability to download its file", "Download")
            QQC2.ToolTip.visible: hovered
        }

        Component {
            id: fileDialog

            FileDialog {
                fileMode: FileDialog.SaveFile
                folder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)
                onAccepted: {
                    currentRoom.downloadFile(eventId, file)
                }
            }
        }

        TapHandler {
            acceptedButtons: Qt.RightButton
            onTapped: openFileContext(model, parent)
        }
        TapHandler {
            acceptedButtons: Qt.LeftButton
            onLongPressed: openFileContext(model, parent)
        }
    }
}
