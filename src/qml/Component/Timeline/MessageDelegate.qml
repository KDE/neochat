// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import Qt.labs.qmlmodels 1.0
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

/**
 * @brief A timeline delegate for an text message.
 *
 * @inherit TimelineContainer
 */
TimelineContainer {
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

    onOpenContextMenu: openMessageContext(label.selectedText)

    Accessible.role: Accessible.StaticText
    Accessible.name: author.displayName + " " + root.display

    innerObject: ColumnLayout {
        Accessible.ignored: true
        Layout.maximumWidth: root.contentMaxWidth
        RichLabel {
            id: label
            Layout.fillWidth: true
            visible: currentRoom.chatBoxEditId !== root.eventId

            isReply: root.isReply

            textMessage: root.display
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
            active: !currentRoom.usesEncryption && currentRoom.urlPreviewEnabled && Config.showLinkPreview && root.showLinkPreview
            linkPreviewer: root.linkPreview
            indicatorEnabled: root.isVisibleInTimeline()
        }
    }
}
