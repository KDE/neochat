// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import Qt.labs.platform 1.1 as Labs
import QtQuick 2.15
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.15 as Kirigami

Labs.Menu {
    id: editMenu

    required property Item field

    Labs.MenuItem {
        enabled: editMenu.field !== null && editMenu.field.canUndo
        text: i18nc("text editing menu action", "Undo")
        shortcut: StandardKey.Undo
        onTriggered: {
            editMenu.field.undo()
            editMenu.close()
        }
    }

    Labs.MenuItem {
        enabled: editMenu.field !== null && editMenu.field.canRedo
        text: i18nc("text editing menu action", "Redo")
        shortcut: StandardKey.Redo
        onTriggered: {
            editMenu.field.undo()
            editMenu.close()
        }
    }

    Labs.MenuSeparator {
    }

    Labs.MenuItem {
        enabled: editMenu.field !== null && editMenu.field.selectedText
        text: i18nc("text editing menu action", "Cut")
        shortcut: StandardKey.Cut
        onTriggered: {
            editMenu.field.cut()
            editMenu.close()
        }
    }

    Labs.MenuItem {
        enabled: editMenu.field !== null && editMenu.field.selectedText
        text: i18nc("text editing menu action", "Copy")
        shortcut: StandardKey.Copy
        onTriggered: {
            editMenu.field.copy()
            editMenu.close()
        }
    }

    Labs.MenuItem {
        enabled: editMenu.field !== null && editMenu.field.canPaste
        text: i18nc("text editing menu action", "Paste")
        shortcut: StandardKey.Paste
        onTriggered: {
            editMenu.field.paste()
            editMenu.close()
        }
    }

    Labs.MenuItem {
        enabled: editMenu.field !== null && editMenu.field.selectedText !== ""
        text: i18nc("text editing menu action", "Delete")
        shortcut: ""
        onTriggered: {
            editMenu.field.remove(editMenu.field.selectionStart, editMenu.field.selectionEnd)
            editMenu.close()
        }
    }

    Labs.MenuSeparator {
    }

    Labs.MenuItem {
        enabled: editMenu.field !== null
        text: i18nc("text editing menu action", "Select All")
        shortcut: StandardKey.SelectAll
        onTriggered: {
            editMenu.field.selectAll()
            editMenu.close()
        }
    }
}
