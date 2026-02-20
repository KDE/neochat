// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-3.0-only

pragma ComponentBehavior: Bound

import QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component to show the image from a message.
 */
Item {
    id: root

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The display text of the message.
     */
    required property string display

    /**
     * @brief The attributes of the component.
     */
    required property var componentAttributes

    /**
     * @brief FileTransferInfo for any downloading files.
     */
    required property var fileTransferInfo

    /**
     * @brief Whether the component should be editable.
     */
    required property bool editable

    /**
     * The maximum height of the image. Can be left undefined most of the times. Passed to MediaSizeHelper::contentMaxHeight.
     */
    property var contentMaxHeight: editable ? Kirigami.Units.gridUnit * 8 : undefined

    /**
     * @brief Extra margin required when anchoring an item on the right.
     *
     * Normally used for scrollbars.
     */
    property int rightAnchorMargin: 0

    Layout.fillWidth: true
    implicitWidth: mediaSizeHelper.currentSize.width
    implicitHeight: mediaSizeHelper.currentSize.height

    RowLayout {
        anchors.top: root.top
        anchors.topMargin: Kirigami.Units.smallSpacing
        anchors.right: root.right
        anchors.rightMargin: root.rightAnchorMargin + Kirigami.Units.smallSpacing

        z: 10

        QQC2.Button {
            visible: !_private.hideImage && !root.editable
            icon.name: "view-hidden"
            text: i18nc("@action:button", "Hide Image")
            display: QQC2.Button.IconOnly
            z: 10
            onClicked: {
                _private.hideImage = true;
                Controller.markImageHidden(root.eventId)
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.Button {
            id: editImageButton
            visible: root.editable
            icon.name: "document-edit"
            text: i18n("Edit")
            display: QQC2.AbstractButton.IconOnly

            Component {
                id: imageEditorPage
                ImageEditorPage {
                    imagePath: root.componentAttributes.source
                }
            }

            onClicked: {
                let imageEditor = (Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(imageEditorPage);
                imageEditor.newPathChanged.connect(function (newPath) {
                    imageEditor.closeDialog();
                    Message.contentModel?.addAttachment(newPath);
                });
            }
            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
        }
        QQC2.Button {
            id: cancelButton
            visible: root.editable
            display: QQC2.AbstractButton.IconOnly
            text: i18nc("@action:button", "Remove attachment")
            icon.name: "dialog-close"
            onClicked: root.Message.contentModel?.removeAttachment()
            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }

    Rectangle {
        anchors.fill: parent

        visible: (_private.imageItem?.status !== Image.Ready ?? true) || _private.hideImage

        color: "#BB000000"

        QQC2.ProgressBar {
            anchors.centerIn: parent

            width: parent.width * 0.8
            visible: !_private.hideImage

            from: 0
            to: 1.0
            value: _private.imageItem?.progress ?? 0.0
        }

        Image {
            anchors.fill: parent
            source: root?.componentAttributes.tempInfo?.source ?? ""
        }
    }

    Loader {
        id: imageLoader

        anchors.fill: parent

        active: !root.componentAttributes.animated && !_private.hideImage
        sourceComponent: Image {
            source: root.componentAttributes.source
            sourceSize.width: mediaSizeHelper.currentSize.width * Screen.devicePixelRatio
            sourceSize.height: mediaSizeHelper.currentSize.height * Screen.devicePixelRatio

            fillMode: Image.PreserveAspectFit
            autoTransform: true
        }
    }

    Loader {
        id: animatedImageLoader

        anchors.fill: parent

        active: (root?.componentAttributes.animated ?? false) && !_private.hideImage
        sourceComponent: AnimatedImage {
            source: root.componentAttributes.source

            fillMode: Image.PreserveAspectFit
            autoTransform: true

            paused: !QQC2.ApplicationWindow.window.active
        }
    }

    QQC2.ToolTip.text: root.display
    QQC2.ToolTip.visible: hoverHandler.hovered
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    HoverHandler {
        id: hoverHandler
    }

    QQC2.Button {
        anchors.centerIn: parent
        text: i18nc("@action:button", "Show Image")
        visible: _private.hideImage
        onClicked: {
            _private.hideImage = false;
            Controller.markImageShown(root.eventId);
        }
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds
        onTapped: {
            root.QQC2.ToolTip.hide();
            if (root.componentAttributes.animated) {
                _private.imageItem.paused = true;
            }
            if (root.Message.timeline) {
                root.Message.timeline.interactive = false;
            }
            if (!root.componentAttributes.isSticker && !root.editable && !_private.hideImage) {
                RoomManager.maximizeMedia(root.eventId);
            }
        }
    }

    function downloadAndOpen() {
        if (_private.downloaded) {
            openSavedFile();
        } else {
            root.openOnFinished = true;
            Message.room.downloadFile(root.eventId, StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + root.eventId.replace(":", "_").replace("/", "_").replace("+", "_") + Message.room.fileNameToDownload(root.eventId));
        }
    }

    function openSavedFile() {
        if (UrlHelper.openUrl(root.fileTransferInfo.localPath))
            return;
        if (UrlHelper.openUrl(root.fileTransferInfo.localDir))
            return;
    }

    MediaSizeHelper {
        id: mediaSizeHelper
        contentMaxWidth: root.Message.maxContentWidth
        contentMaxHeight: root.contentMaxHeight ?? -1
        mediaWidth: root?.componentAttributes.isSticker ? 256 : (root?.componentAttributes.width ?? 0)
        mediaHeight: root?.componentAttributes.isSticker ? 256 : (root?.componentAttributes.height ?? 0)
    }

    QtObject {
        id: _private
        readonly property var imageItem: root.componentAttributes.animated ? animatedImageLoader.item : imageLoader.item

        // The space available for the component after taking away the border
        readonly property real downloaded: root.fileTransferInfo && root.fileTransferInfo.completed

        property bool hideImage: NeoChatConfig.hideImages && !Controller.isImageShown(root.eventId)
    }
}
