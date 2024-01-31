// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import Qt.labs.platform as Labs
import QtQuick
import QtQuick.Layouts

Labs.Menu {
    id: root

    required property Item field

    Labs.MenuItem {
        enabled: root.field !== null && root.field.canUndo
        text: i18nc("text editing menu action", "Undo")
        shortcut: StandardKey.Undo
        onTriggered: {
            root.field.undo();
            root.close();
        }
    }

    Labs.MenuItem {
        enabled: root.field !== null && root.field.canRedo
        text: i18nc("text editing menu action", "Redo")
        shortcut: StandardKey.Redo
        onTriggered: {
            root.field.undo();
            root.close();
        }
    }

    Labs.MenuSeparator {}

    Labs.MenuItem {
        enabled: root.field !== null && root.field.selectedText
        text: i18nc("text editing menu action", "Cut")
        shortcut: StandardKey.Cut
        onTriggered: {
            root.field.cut();
            root.close();
        }
    }

    Labs.MenuItem {
        enabled: root.field !== null && root.field.selectedText
        text: i18nc("text editing menu action", "Copy")
        shortcut: StandardKey.Copy
        onTriggered: {
            root.field.copy();
            root.close();
        }
    }

    Labs.MenuItem {
        enabled: root.field !== null && root.field.canPaste
        text: i18nc("text editing menu action", "Paste")
        shortcut: StandardKey.Paste
        onTriggered: {
            root.field.paste();
            root.close();
        }
    }

    Labs.MenuItem {
        enabled: root.field !== null && root.field.selectedText !== ""
        text: i18nc("text editing menu action", "Delete")
        shortcut: ""
        onTriggered: {
            root.field.remove(root.field.selectionStart, root.field.selectionEnd);
            root.close();
        }
    }

    Labs.MenuSeparator {}

    Labs.MenuItem {
        enabled: root.field !== null
        text: i18nc("text editing menu action", "Select All")
        shortcut: StandardKey.SelectAll
        onTriggered: {
            root.field.selectAll();
            root.close();
        }
    }
}
