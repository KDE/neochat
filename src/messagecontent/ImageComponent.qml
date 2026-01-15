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
     * @brief The Blocks::Block for the delegate.
     */
    required property ImageBlock block

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

    /**
     * @brief Whether the media should be hidden.
     */
    required property bool mediaHidden

    Layout.fillWidth: true
    implicitWidth: container.implicitWidth
    implicitHeight: container.implicitHeight

    Item {
        id: container
        implicitWidth: mediaSizeHelper.currentSize.width
        implicitHeight: mediaSizeHelper.currentSize.height

        RowLayout {
            anchors.top: parent.top
            anchors.topMargin: Kirigami.Units.smallSpacing
            anchors.right: parent.right
            anchors.rightMargin: root.rightAnchorMargin + Kirigami.Units.smallSpacing

            z: 10

            QQC2.Button {
                id: hideButton

                // For tiny images, having the button looks super buggy at their size
                visible: !root.mediaHidden && !root.editable && root.width >= hideButton.width && root.height >= hideButton.height
                icon.name: "view-hidden"
                text: i18nc("@action:button", "Hide Image")
                display: QQC2.Button.IconOnly
                z: 10
                onClicked: Message.contentModel?.hideMedia()

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            QQC2.Button {
                id: editImageButton
                visible: root.editable
                icon.name: "document-edit"
                text: i18nc("@action:button", "Edit")
                display: QQC2.AbstractButton.IconOnly

                Component {
                    id: imageEditorPage
                    ImageEditorPage {
                        imagePath: root.block.source
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

            visible: (_private.imageItem?.status !== Image.Ready ?? true) || root.mediaHidden

            color: "#BB000000"

            QQC2.ProgressBar {
                anchors.centerIn: parent

                width: parent.width * 0.8
                visible: !root.mediaHidden

                from: 0
                to: 1.0
                value: _private.imageItem?.progress ?? 0.0
            }

            Image {
                anchors.fill: parent
                source: root?.block.thumbnailSource
            }
        }

        Loader {
            id: imageLoader

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left

            active: !root.block.info.isAnimated && !root.mediaHidden
            sourceComponent: Image {
                source: root.block.source
                sourceSize.width: mediaSizeHelper.currentSize.width * Screen.devicePixelRatio
                sourceSize.height: mediaSizeHelper.currentSize.height * Screen.devicePixelRatio

                fillMode: Image.PreserveAspectFit
                autoTransform: true
            }
        }

        Loader {
            id: animatedImageLoader

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left

            active: (root?.block.info.isAnimated ?? false) && !root.mediaHidden
            sourceComponent: AnimatedImage {
                source: root.block.source

                fillMode: Image.PreserveAspectFit
                autoTransform: true

                paused: !QQC2.ApplicationWindow.window.active
            }
        }

        HoverHandler {
            id: hoverHandler
        }

        QQC2.Button {
            anchors.centerIn: parent
            text: i18nc("@action:button", "Show Image")
            visible: root.mediaHidden
            onClicked: Message.contentModel?.showMedia()
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds
            onTapped: {
                root.QQC2.ToolTip.hide();
                if (root.block.info.isAnimated) {
                    _private.imageItem.paused = true;
                }
                if (root.Message.timeline) {
                    root.Message.timeline.interactive = false;
                }
                if (!root.block.info.isSticker && !root.editable && !root.mediaHidden) {
                    RoomManager.maximizeMedia(root.eventId);
                }
            }
        }
    }

    QQC2.ToolTip.text: root.block.filename
    QQC2.ToolTip.visible: hoverHandler.hovered
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    MediaSizeHelper {
        id: mediaSizeHelper
        contentMaxWidth: root.Message.maxContentWidth
        contentMaxHeight: root.contentMaxHeight ?? -1
        mediaWidth: root?.block.info.isSticker ? 256 : (root?.block.info.pixelSize.width ?? 0)
        mediaHeight: root?.block.info.isSticker ? 256 : (root?.block.info.pixelSize.height ?? 0)
    }

    QtObject {
        id: _private
        readonly property var imageItem: root.block.info.isAnimated ? animatedImageLoader.item : imageLoader.item
    }
}
