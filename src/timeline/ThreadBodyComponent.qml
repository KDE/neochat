// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component to visualize a ThreadModel.
 *
 * @sa ThreadModel
 */
ColumnLayout {
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
     * @brief The Matrix ID of the root message in the thread, if any.
     */
    required property string threadRoot

    /**
     * @brief The timeline ListView this component is being used in.
     */
    required property ListView timeline

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    /**
     * @brief The reply has been clicked.
     */
    signal replyClicked(string eventID)

    /**
     * @brief The user selected text has changed.
     */
    signal selectedTextChanged(string selectedText)

    /**
     * @brief The user hovered link has changed.
     */
    signal hoveredLinkChanged(string hoveredLink)

    /**
     * @brief Request a context menu be show for the message.
     */
    signal showMessageMenu

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.maximumWidth: root.maxContentWidth
    spacing: Kirigami.Units.smallSpacing

    Repeater {
        id: threadRepeater
        model: root.room.modelForThread(root.threadRoot);

        delegate: BaseMessageComponentChooser {
            room: root.room
            index: root.index
            timeline: root.timeline
            maxContentWidth: root.maxContentWidth

            onReplyClicked: eventId => {
                root.replyClicked(eventId);
            }
            onSelectedTextChanged: selectedText => {
                root.selectedTextChanged(selectedText);
            }
            onHoveredLinkChanged: hoveredLink => {
                root.hoveredLinkChanged(hoveredLink);
            }
            onShowMessageMenu: root.showMessageMenu()
            onRemoveLinkPreview: index => threadRepeater.model.closeLinkPreview(index)
            onFetchMoreEvents: threadRepeater.model.fetchMoreEvents(5)
        }
    }
}
