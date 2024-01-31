// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.platform
import Qt.labs.qmlmodels

import org.kde.coreaddons
import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.config

/**
 * @brief A timeline delegate for an file message.
 *
 * @inherit MessageDelegate
 */
MessageDelegate {
    id: root

    /**
     * @brief The media info for the event.
     *
     * This should consist of the following:
     *  - source - The mxc URL for the media.
     *  - mimeType - The MIME type of the media.
     *  - mimeIcon - The MIME icon name.
     *  - size - The file size in bytes.
     */
    required property var mediaInfo

    /**
     * @brief Whether the media has been downloaded.
     */
    readonly property bool downloaded: root.progressInfo && root.progressInfo.completed

    /**
     * @brief Whether the file should be automatically opened when downloaded.
     */
    property bool autoOpenFile: false

    onDownloadedChanged: {
        itineraryModel.path = root.progressInfo.localPath;
        if (autoOpenFile) {
            openSavedFile();
        }
    }

    onOpenContextMenu: RoomManager.viewEventMenu(eventId, author, delegateType, plainText, "", "", mediaInfo.mimeType, progressInfo)

    function saveFileAs() {
        const dialog = fileDialog.createObject(QQC2.ApplicationWindow.overlay);
        dialog.open();
        dialog.currentFile = dialog.folder + "/" + root.room.fileNameToDownload(root.eventId);
    }

    function openSavedFile() {
        UrlHelper.openUrl(root.progressInfo.localPath);
    }

    bubbleContent: ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            states: [
                State {
                    name: "downloadedInstant"
                    when: root.progressInfo.completed && autoOpenFile

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
                    when: root.progressInfo.completed && !autoOpenFile

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
                    when: root.progressInfo.active

                    PropertyChanges {
                        target: openButton
                        visible: false
                    }

                    PropertyChanges {
                        target: sizeLabel
                        text: i18nc("file download progress", "%1 / %2", Format.formatByteSize(root.progressInfo.progress), Format.formatByteSize(root.progressInfo.total))
                    }
                    PropertyChanges {
                        target: downloadButton
                        icon.name: "media-playback-stop"
                        QQC2.ToolTip.text: i18nc("tooltip for a button on a message; stops downloading the message's file", "Stop Download")
                        onClicked: root.room.cancelFileTransfer(root.eventId)
                    }
                },
                State {
                    name: "raw"
                    when: true

                    PropertyChanges {
                        target: downloadButton
                        onClicked: root.saveFileAs()
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
                    text: root.display
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
                    root.room.downloadTempFile(root.eventId);
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
                        Config.lastSaveDirectory = folder;
                        Config.save();
                        if (autoOpenFile) {
                            UrlHelper.copyTo(root.progressInfo.localPath, file);
                        } else {
                            root.room.download(root.eventId, file);
                        }
                    }
                }
            }
        }
        Repeater {
            id: itinerary
            model: ItineraryModel {
                id: itineraryModel
                connection: root.room.connection
            }
            delegate: DelegateChooser {
                role: "type"
                DelegateChoice {
                    roleValue: "TrainReservation"
                    delegate: ColumnLayout {
                        Kirigami.Separator {
                            Layout.fillWidth: true
                        }
                        RowLayout {
                            QQC2.Label {
                                text: model.name
                            }
                            QQC2.Label {
                                text: model.coach ? i18n("Coach: %1, Seat: %2", model.coach, model.seat) : ""
                                visible: model.coach
                                opacity: 0.7
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            ColumnLayout {
                                QQC2.Label {
                                    text: model.departureStation + (model.departurePlatform ? (" [" + model.departurePlatform + "]") : "")
                                }
                                QQC2.Label {
                                    text: model.departureTime
                                    opacity: 0.7
                                }
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            ColumnLayout {
                                QQC2.Label {
                                    text: model.arrivalStation + (model.arrivalPlatform ? (" [" + model.arrivalPlatform + "]") : "")
                                }
                                QQC2.Label {
                                    text: model.arrivalTime
                                    opacity: 0.7
                                    Layout.alignment: Qt.AlignRight
                                }
                            }
                        }
                    }
                }
                DelegateChoice {
                    roleValue: "LodgingReservation"
                    delegate: ColumnLayout {
                        Kirigami.Separator {
                            Layout.fillWidth: true
                        }
                        QQC2.Label {
                            text: model.name
                        }
                        QQC2.Label {
                            text: i18nc("<start time> - <end time>", "%1 - %2", model.startTime, model.endTime)
                        }
                        QQC2.Label {
                            text: model.address
                        }
                    }
                }
            }
        }
        QQC2.Button {
            icon.name: "map-globe"
            text: i18nc("@action", "Send to KDE Itinerary")
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.text: text
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            onClicked: itineraryModel.sendToItinerary()
            visible: itinerary.count > 0
        }
    }
}
