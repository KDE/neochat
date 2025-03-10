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
     * @brief The Matrix ID of the root message in the thread, if any.
     */
    required property string threadRoot

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
    Layout.maximumWidth: Message.maxContentWidth
    spacing: Kirigami.Units.smallSpacing

    Repeater {
        id: threadRepeater
        model: root.Message.room.modelForThread(root.threadRoot);

        delegate: BaseMessageComponentChooser {
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
