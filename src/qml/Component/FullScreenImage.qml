// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

QQC2.Popup {
    id: root

    property alias source: image.source
    property string filename
    property string blurhash: ""
    property int imageWidth: -1
    property int imageHeight: -1
    property var modelData

    parent: QQC2.Overlay.overlay
    closePolicy: QQC2.Popup.CloseOnEscape
    width: parent.width
    height: parent.height
    modal: true
    padding: 0
    background: null

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.largeSpacing

        QQC2.Control {
            Layout.fillWidth: true

            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Avatar {
                    id: avatar

                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium

                    name: modelData.author.name ?? modelData.author.displayName
                    source: modelData.author.avatarMediaId ? ("image://mxc/" + modelData.author.avatarMediaId) : ""
                    color: modelData.author.color
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    QQC2.Label {
                        id: nameLabel

                        text: modelData.author.displayName
                        textFormat: Text.PlainText
                        font.weight: Font.Bold
                        color: author.color
                    }
                    QQC2.Label {
                        id: timeLabel

                        text: time.toLocaleString(Qt.locale(), Locale.ShortFormat)
                    }
                }
                QQC2.Label {
                    id: imageLabel
                    Layout.fillWidth: true
                    Layout.leftMargin: Kirigami.Units.largeSpacing

                    text: modelData.display
                    font.weight: Font.Bold
                    elide: Text.ElideRight
                }
                QQC2.ToolButton {
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 2

                    text: i18n("Zoom in")
                    Accessible.name: text
                    icon.name: "zoom-in"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: {
                        image.scaleFactor = image.scaleFactor + 0.25
                        if (image.scaleFactor > 3) {
                            image.scaleFactor = 3
                        }
                    }

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                }
                QQC2.ToolButton {
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 2

                    text: i18n("Zoom out")
                    Accessible.name: text
                    icon.name: "zoom-out"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: {
                        image.scaleFactor = image.scaleFactor - 0.25
                        if (image.scaleFactor < 0.25) {
                            image.scaleFactor = 0.25
                        }
                    }

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                }
                QQC2.ToolButton {
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 2

                    text: i18n("Rotate left")
                    Accessible.name: text
                    icon.name: "image-rotate-left-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: image.rotationAngle = image.rotationAngle - 90

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                }
                QQC2.ToolButton {
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 2

                    text: i18n("Rotate right")
                    Accessible.name: text
                    icon.name: "image-rotate-right-symbolic"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: image.rotationAngle = image.rotationAngle + 90

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                }
                QQC2.ToolButton {
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 2

                    text: i18n("Save as")
                    Accessible.name: text
                    icon.name: "document-save"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: {
                        var dialog = saveAsDialog.createObject(QQC2.ApplicationWindow.overlay)
                        dialog.open()
                        dialog.currentFile = dialog.folder + "/" + currentRoom.fileNameToDownload(eventId)
                    }

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                }
                QQC2.ToolButton {
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 2

                    text: i18n("Close")
                    Accessible.name: text
                    icon.name: "dialog-close"
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: {
                        root.close()
                    }

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                }
            }

            background: Rectangle {
                color: Kirigami.Theme.alternateBackgroundColor
            }

            Kirigami.Separator {
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                height: 1
            }
        }

        QQC2.BusyIndicator {
            Layout.fillWidth: true
            visible: image.status !== Image.Ready && root.blurhash === ""
            running: visible
        }
        // Provides container to fill the space that isn't taken up by the top controls and clips the image when zooming makes it larger than the available area.
        Item {
            id: imageContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            clip: true

            AnimatedImage {
                id: image

                property var scaleFactor: 1
                property int rotationAngle: 0
                property var rotationInsensitiveWidth: Math.min(root.imageWidth > 0 ? root.imageWidth : sourceSize.width, imageContainer.width - Kirigami.Units.largeSpacing * 2)
                property var rotationInsensitiveHeight: Math.min(root.imageHeight > 0 ? root.imageHeight : sourceSize.height, imageContainer.height - Kirigami.Units.largeSpacing * 2)

                anchors.centerIn: parent
                width: rotationAngle % 180 === 0 ? rotationInsensitiveWidth : rotationInsensitiveHeight
                height: rotationAngle % 180 === 0 ? rotationInsensitiveHeight : rotationInsensitiveWidth
                fillMode: Image.PreserveAspectFit
                clip: true

                Behavior on width {
                    NumberAnimation {duration: Kirigami.Units.longDuration; easing.type: Easing.InOutCubic}
                }
                Behavior on height {
                    NumberAnimation {duration: Kirigami.Units.longDuration; easing.type: Easing.InOutCubic}
                }

                Image {
                    anchors.centerIn: parent
                    width: image.width
                    height: image.height
                    source: root.blurhash !== "" ? ("image://blurhash/" + root.blurhash) : ""
                    visible: root.blurhash !== "" && parent.status !== Image.Ready
                }

                transform: [
                    Rotation {
                        origin.x: image.width / 2
                        origin.y: image.height / 2
                        angle: image.rotationAngle

                        Behavior on angle {
                            RotationAnimation {duration: Kirigami.Units.longDuration; easing.type: Easing.InOutCubic}
                        }
                    },
                    Scale {
                        origin.x: image.width / 2
                        origin.y: image.height / 2
                        xScale: image.scaleFactor
                        yScale: image.scaleFactor

                        Behavior on xScale {
                            NumberAnimation {duration: Kirigami.Units.longDuration; easing.type: Easing.InOutCubic}
                        }
                        Behavior on yScale {
                            NumberAnimation {duration: Kirigami.Units.longDuration; easing.type: Easing.InOutCubic}
                        }
                    }
                ]

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    onClicked: {
                        const contextMenu = fileDelegateContextMenu.createObject(parent, {
                            author: modelData.author,
                            message: modelData.message,
                            eventId: modelData.eventId,
                            source: modelData.source,
                            file: root.parent,
                            mimeType: modelData.mimeType,
                            progressInfo: modelData.progressInfo,
                            plainMessage: modelData.message,
                        });
                        contextMenu.closeFullscreen.connect(root.close)
                        contextMenu.open();
                    }
                }
            }
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onClicked: {
                    root.close()
                }
            }
        }
    }

    Component {
        id: saveAsDialog
        FileDialog {
            fileMode: FileDialog.SaveFile
            folder: Config.lastSaveDirectory.length > 0 ? Config.lastSaveDirectory : StandardPaths.writableLocation(StandardPaths.DownloadLocation)
            onAccepted: {
                Config.lastSaveDirectory = folder
                Config.save()
                if (!currentFile) {
                    return;
                }
                currentRoom.downloadFile(eventId, currentFile)
            }
        }
    }

    onClosed: {
        image.scaleFactor = 1
        image.rotationAngle = 0
    }
}
