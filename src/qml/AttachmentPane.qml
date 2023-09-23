// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat

ColumnLayout {
    id: root

    signal attachmentCancelled()

    property string attachmentPath

    readonly property var attachmentMimetype: FileType.mimeTypeForUrl(attachmentPath)
    readonly property bool hasImage: attachmentMimetype.valid && FileType.supportedImageFormats.includes(attachmentMimetype.preferredSuffix)
    readonly property string baseFileName: attachmentPath.substring(attachmentPath.lastIndexOf('/') + 1, attachmentPath.length)

    RowLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft
            text: i18n("Attachment:")
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
        QQC2.ToolButton {
            id: editImageButton
            visible: hasImage
            icon.name: "document-edit"
            text: i18n("Edit")
            display: QQC2.AbstractButton.IconOnly

            Component {
                id: imageEditorPage
                ImageEditorPage {
                    imagePath: root.attachmentPath
                }
            }

            onClicked: {
                let imageEditor = applicationWindow().pageStack.layers.push(imageEditorPage);
                imageEditor.newPathChanged.connect(function(newPath) {
                    applicationWindow().pageStack.layers.pop();
                    root.attachmentPath = newPath;
                });
            }
            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
        }
        QQC2.ToolButton {
            id: cancelAttachmentButton
            display: QQC2.AbstractButton.IconOnly
            action: Kirigami.Action {
                text: i18n("Cancel sending attachment")
                icon.name: "dialog-close"
                onTriggered: attachmentCancelled();
                shortcut: "Escape"
            }
            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
        }
    }

    Image {
        id: image
        Layout.alignment: Qt.AlignHCenter

        asynchronous: true
        cache: false // Cache is not needed. Images will rarely be shown repeatedly.
        source: hasImage ? root.attachmentPath : ""
        visible: hasImage
        fillMode: Image.PreserveAspectFit

        onSourceChanged: {
            // Reset source size height, which affect implicitHeight
            sourceSize.height = -1
        }

        onSourceSizeChanged: {
            if (implicitHeight > Kirigami.Units.gridUnit * 8) {
                // This can save a lot of RAM when loading large images.
                // It also improves visual quality for large images.
                sourceSize.height = Kirigami.Units.gridUnit * 8
            }
        }

        Behavior on height {
            NumberAnimation {
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }
    }
    QQC2.BusyIndicator {
        id: imageBusyIndicator

        visible: running
        running: image.visible && image.progress < 1
    }
    RowLayout {
        id: fileInfoLayout
        Layout.alignment: Qt.AlignHCenter
        spacing: parent.spacing

        Kirigami.Icon {
            id: mimetypeIcon
            implicitWidth: Kirigami.Units.iconSizes.smallMedium
            implicitHeight: Kirigami.Units.iconSizes.smallMedium
            source: attachmentMimetype.iconName
        }
        QQC2.Label {
            id: fileLabel
            text: baseFileName
        }
    }
}
