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
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief The index of the delegate in the model.
     */
    required property var index

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
     * @brief The timeline ListView this component is being used in.
     */
    required property ListView timeline

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    implicitWidth: mediaSizeHelper.currentSize.width
    implicitHeight: mediaSizeHelper.currentSize.height

    Loader {
        id: imageLoader

        anchors.fill: parent

        active: !root.mediaInfo.animated
        sourceComponent: Image {
            source: root.mediaInfo.source
            sourceSize.width: mediaSizeHelper.currentSize.width * Screen.devicePixelRatio
            sourceSize.height: mediaSizeHelper.currentSize.height * Screen.devicePixelRatio

            fillMode: Image.PreserveAspectFit
        }
    }

    Loader {
        id: animatedImageLoader

        anchors.fill: parent

        active: root?.mediaInfo.animated ?? false
        sourceComponent: AnimatedImage {
            source: root.mediaInfo.source

            fillMode: Image.PreserveAspectFit

            paused: !applicationWindow().active
        }
    }

    Image {
        anchors.fill: parent
        source: root?.mediaInfo.tempInfo.source ?? ""
        visible: _private.imageItem.status !== Image.Ready
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

            from: 0
            to: 1.0
            value: _private.imageItem.progress
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
            root.timeline.interactive = false;
            if (!root.mediaInfo.isSticker) {
                // We need to make sure the index is that of the MediaMessageFilterModel.
                if (root.timeline.model instanceof MessageFilterModel) {
                    RoomManager.maximizeMedia(RoomManager.mediaMessageFilterModel.getRowForSourceItem(root.index));
                } else {
                    RoomManager.maximizeMedia(root.index);
                }
            }
        }
    }

    function downloadAndOpen() {
        if (_private.downloaded) {
            openSavedFile();
        } else {
            openOnFinished = true;
            root.room.downloadFile(root.eventId, StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + root.eventId.replace(":", "_").replace("/", "_").replace("+", "_") + root.room.fileNameToDownload(root.eventId));
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
        contentMaxWidth: root.maxContentWidth
        mediaWidth: root?.mediaInfo.width ?? 0
        mediaHeight: root?.mediaInfo.height ?? 0
    }

    QtObject {
        id: _private
        readonly property var imageItem: root.mediaInfo.animated ? animatedImageLoader.item : imageLoader.item

        // The space available for the component after taking away the border
        readonly property real downloaded: root.fileTransferInfo && root.fileTransferInfo.completed
    }
}
