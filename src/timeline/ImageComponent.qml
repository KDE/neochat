// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-3.0-only

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
     *  - isSticker - Whether the image is a sticker or not
     */
    required property var mediaInfo

    /**
     * @brief FileTransferInfo for any downloading files.
     */
    required property var fileTransferInfo

    /**
     * The maximum height of the image. Can be left undefined most of the times. Passed to MediaSizeHelper::contentMaxHeight.
     */
    property int contentMaxHeight: undefined

    implicitWidth: mediaSizeHelper.currentSize.width
    implicitHeight: mediaSizeHelper.currentSize.height

    Loader {
        id: imageLoader

        anchors.fill: parent

        active: !root.mediaInfo.animated && !_private.hideImage
        sourceComponent: Image {
            source: root.mediaInfo.source
            sourceSize.width: mediaSizeHelper.currentSize.width * Screen.devicePixelRatio
            sourceSize.height: mediaSizeHelper.currentSize.height * Screen.devicePixelRatio

            fillMode: Image.PreserveAspectFit
            autoTransform: true
        }
    }

    Loader {
        id: animatedImageLoader

        anchors.fill: parent

        active: (root?.mediaInfo.animated ?? false) && !_private.hideImage
        sourceComponent: AnimatedImage {
            source: root.mediaInfo.source

            fillMode: Image.PreserveAspectFit
            autoTransform: true

            paused: !applicationWindow().active
        }
    }

    Image {
        anchors.fill: parent
        source: visible ? (root?.mediaInfo.tempInfo?.source ?? "") : ""
        visible: _private.imageItem && _private.imageItem.status !== Image.Ready && !_private.hideImage
    }

    QQC2.ToolTip.text: root.display
    QQC2.ToolTip.visible: hoverHandler.hovered
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    HoverHandler {
        id: hoverHandler
    }

    Rectangle {
        anchors.fill: parent

        visible: _private.imageItem.status !== Image.Ready

        color: "#BB000000"

        QQC2.ProgressBar {
            anchors.centerIn: parent

            width: parent.width * 0.8
            visible: !_private.hideImage

            from: 0
            to: 1.0
            value: _private.imageItem.progress
        }

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
            if (root.mediaInfo.animated) {
                _private.imageItem.paused = true;
            }
            root.Message.timeline.interactive = false;
            if (!root.mediaInfo.isSticker) {
                // We need to make sure the index is that of the MediaMessageFilterModel.
                if (root.Message.timeline.model instanceof MessageFilterModel) {
                    RoomManager.maximizeMedia(RoomManager.mediaMessageFilterModel.getRowForSourceItem(root.Message.index));
                } else {
                    RoomManager.maximizeMedia(root.Message.index);
                }
            }
        }
    }

    function downloadAndOpen() {
        if (_private.downloaded) {
            openSavedFile();
        } else {
            openOnFinished = true;
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
        contentMaxHeight: root.contentMaxHeight
        mediaWidth: root?.mediaInfo.isSticker ? 256 : (root?.mediaInfo.width ?? 0)
        mediaHeight: root?.mediaInfo.isSticker ? 256 : (root?.mediaInfo.height ?? 0)
    }

    QtObject {
        id: _private
        readonly property var imageItem: root.mediaInfo.animated ? animatedImageLoader.item : imageLoader.item

        // The space available for the component after taking away the border
        readonly property real downloaded: root.fileTransferInfo && root.fileTransferInfo.completed

        property bool hideImage: NeoChatConfig.hideImages && !Controller.isImageShown(root.eventId)
    }
}
