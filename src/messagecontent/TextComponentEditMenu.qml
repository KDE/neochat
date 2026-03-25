// SPDX-FileCopyrightText: 2020 Devin Lin <espidev@gmail.com>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQml.Models
import QtQuick
import org.kde.desktop as QQC2

import org.kde.kirigami as Kirigami
import org.kde.sonnet as Sonnet

QQC2.Menu {
    id: root

    required property TextEdit target
    required property int openPoint
    property int restoredCursorPosition: 0
    property int restoredSelectionStart
    property int restoredSelectionEnd

    required property Sonnet.SpellcheckHighlighter spellcheckHighlighter

    readonly property Sonnet.Settings settings: Sonnet.Settings {}

    property var runOnMenuClose: () => {}

    function storeCursorAndSelection() {
        restoredCursorPosition = target.cursorPosition;
        restoredSelectionStart = target.selectionStart;
        restoredSelectionEnd = target.selectionEnd;
    }

    function __hasSelectedText(): bool {
        return target !== null
            && target.selectedText !== "";
    }

    function __showSpellcheckActions(): bool {
        return spellcheckHighlighter !== null &&
               spellcheckHighlighter.active &&
               spellcheckHighlighter.wordIsMisspelled;
    }

    // deal with whether text should be deselected
    onClosed: {
        // restore cursor position
        target.forceActiveFocus();
        target.cursorPosition = restoredCursorPosition;
        target.select(restoredSelectionStart, restoredSelectionEnd);

        // run action
        runOnMenuClose();
        runOnMenuClose = () => {};
    }

    onOpened: {
        suggestionInstantiator.model = spellcheckHighlighter.suggestions(root.openPoint);
        // For whatever reason the text gets deselected when the TextEdit is !readonly
        // even is persistentSelection is true.
        // So reselect manually.
        target.cursorPosition = restoredCursorPosition;
        target.select(restoredSelectionStart, restoredSelectionEnd);
    }

    Instantiator {
        id: suggestionInstantiator
        active: root.__showSpellcheckActions()

        model: root.spellcheckSuggestions
        delegate: QQC2.MenuItem {
            required property string modelData

            text: modelData

            onClicked: {
                root.runOnMenuClose = () => {
                    root.spellcheckHighlighter.replaceWord(modelData);
                };
            }
        }
        onObjectAdded: (index, object) => {
            root.insertItem(0, object);
        }
        onObjectRemoved: (index, object) => {
            root.removeItem(object);
        }
    }
    QQC2.MenuItem {
        visible: root.__showSpellcheckActions() && suggestionInstantiator.count === 0
        action: Kirigami.Action {
            enabled: false
            text: root.spellcheckHighlighter ? i18nc("As in no suggestion for spellcheck to fix spelling", "No Suggestions for \"%1\"", root.spellcheckHighlighter.wordUnderMouse) : ""
        }
    }
    QQC2.MenuSeparator {
        visible: root.__showSpellcheckActions()
    }
    QQC2.MenuItem {
        visible: root.__showSpellcheckActions()
        action: Kirigami.Action {
            text: root.spellcheckHighlighter ? i18nc("As in add the word %1 to spellcheck dictionary", "Add \"%1\" to Dictionary", root.spellcheckHighlighter.wordUnderMouse) : ""

            onTriggered: {
                root.runOnMenuClose = () => {
                    root.spellcheckHighlighter.addWordToDictionary(root.spellcheckHighlighter.wordUnderMouse);
                };
            }
        }
    }
    QQC2.MenuItem {
        visible: root.__showSpellcheckActions()
        action: Kirigami.Action {
            text: i18nc("@action:inmenu", "Ignore")
            onTriggered: {
                root.runOnMenuClose = () => {
                    root.spellcheckHighlighter.ignoreWord(root.spellcheckHighlighter.wordUnderMouse);
                };
            }
        }
    }
    QQC2.MenuItem {
        checkable: true
        checked: if (root.spellcheckHighlighter) {
            return root.spellcheckHighlighter.active;
        }
        text: i18nc("@action:inmenu", "Spell Check")

        onToggled: {
            if (root.spellcheckHighlighter) {
                root.spellcheckHighlighter.active = checked;
            }
        }
    }
    QQC2.MenuSeparator {}
    QQC2.MenuItem {
        action: Kirigami.Action {
            icon.name: "edit-undo-symbolic"
            text: i18nc("@action:inmenu", "Undo")
            shortcut: StandardKey.Undo
        }
        enabled: root.target?.canUndo ?? false
        onTriggered: {
            root.runOnMenuClose = () => {
                root.target.undo();
            };
        }
    }
    QQC2.MenuItem {
        action: Kirigami.Action {
            icon.name: "edit-redo-symbolic"
            text: i18nc("@action:inmenu", "Redo")
            shortcut: StandardKey.Redo
        }
        enabled: root.target?.canRedo ?? false
        onTriggered: {
            root.runOnMenuClose = () => {
                root.target.redo();
            };
        }
    }
    QQC2.MenuSeparator {}
    QQC2.MenuItem {
        action: Kirigami.Action {
            icon.name: "edit-cut-symbolic"
            text: i18nc("@action:inmenu", "Cut")
            shortcut: StandardKey.Cut
        }
        enabled: root.__hasSelectedText()
        onTriggered: {
            root.runOnMenuClose = () => {
                root.target.cut();
            };
        }
    }
    QQC2.MenuItem {
        action: Kirigami.Action {
            icon.name: "edit-copy-symbolic"
            text: i18nc("@action:inmenu", "Copy")
            shortcut: StandardKey.Copy
        }
        enabled: root.__hasSelectedText()
        onTriggered: {
            root.runOnMenuClose = () => {
                root.target.copy();
            };
        }
    }
    QQC2.MenuItem {
        action: Kirigami.Action {
            icon.name: "edit-paste-symbolic"
            text: i18nc("@action:inmenu", "Paste")
            shortcut: StandardKey.Paste
        }
        enabled: target?.canPaste ?? false
        onTriggered: {
            root.runOnMenuClose = () => {
                root.target.paste();
            };
        }
    }
    QQC2.MenuItem {
        action: Kirigami.Action {
            icon.name: "edit-delete-symbolic"
            text: i18nc("@action:inmenu", "Delete")
            shortcut: StandardKey.Delete
        }
        enabled: root.__hasSelectedText()
        onTriggered: {
            root.runOnMenuClose = () => {
                root.target.remove(root.target.selectionStart, root.target.selectionEnd);
            };
        }
    }
    QQC2.MenuSeparator {}
    QQC2.MenuItem {
        action: Kirigami.Action {
            icon.name: "edit-select-all-symbolic"
            text: i18nc("@action:inmenu", "Select All")
            shortcut: StandardKey.SelectAll
        }
        visible: root.target !== null
        onTriggered: {
            root.runOnMenuClose = () => {
                root.target.selectAll();
            };
        }
    }
}
