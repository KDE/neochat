// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later


/**
 * Context menu when clicking on a room in the room list
 */
Menu {
    id: root
    property var selectedText

    Repeater {
        model: WebShortcutModel {
            selectedText: root.selectedText
        }
        delegate: MenuItem {
            text: model.display
            icon.name: model.decoration
        }
    }

    MenuSeparator {}

    onClosed: destroy()
}
