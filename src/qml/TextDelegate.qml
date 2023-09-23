// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Layouts

import Qt.labs.qmlmodels

import org.kde.neochat
import org.kde.neochat.config

/**
 * @brief A timeline delegate for a text message.
 *
 * @inherit MessageDelegate
 */
MessageDelegate {
    id: root

    /**
     * @brief The link preview properties.
     *
     * This is a list or object containing the following:
     *  - url - The URL being previewed.
     *  - loaded - Whether the URL preview has been loaded.
     *  - title - the title of the URL preview.
     *  - description - the description of the URL preview.
     *  - imageSource - a source URL for the preview image.
     *
     * @note An empty link previewer should be passed if there are no links to
     *       preview.
     */
    required property var linkPreview

    /**
     * @brief Whether there are any links to preview.
     */
    required property bool showLinkPreview

    onOpenContextMenu: RoomManager.viewEventMenu(eventId, author, delegateType, plainText, display, label.selectedText)

    bubbleContent: ColumnLayout {
        RichLabel {
            id: label
            Layout.fillWidth: true
            visible: currentRoom.chatBoxEditId !== root.eventId

            isReply: root.isReply

            textMessage: root.display

            TapHandler {
                enabled: !label.hoveredLink
                acceptedButtons: Qt.LeftButton
                onLongPressed: root.openContextMenu()
            }
        }
        Loader {
            Layout.fillWidth: true
            Layout.minimumHeight: item ? item.minimumHeight : -1
            Layout.preferredWidth: item ? item.preferredWidth : -1
            visible: currentRoom.chatBoxEditId === root.eventId
            active: visible
            sourceComponent: MessageEditComponent {
                room: currentRoom
                messageId: root.eventId
            }
        }
        LinkPreviewDelegate {
            Layout.fillWidth: true
            active: !currentRoom.usesEncryption && currentRoom.urlPreviewEnabled && Config.showLinkPreview && root.showLinkPreview && !root.linkPreview.empty
            linkPreviewer: root.linkPreview
            indicatorEnabled: root.isVisibleInTimeline()
        }
    }
}
