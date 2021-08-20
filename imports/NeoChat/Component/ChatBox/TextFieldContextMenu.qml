/*
    SPDX-FileCopyrightText: 2020 Devin Lin <espidev@gmail.com>
    SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/ 

pragma Singleton

import QtQuick 2.6
import QtQml 2.2
import QtQuick.Controls 2.15
import org.kde.kirigami 2.5 as Kirigami

Menu {
    id: contextMenu

    property Item target
    property bool deselectWhenMenuClosed: true
    property int restoredCursorPosition: 0
    property int restoredSelectionStart
    property int restoredSelectionEnd
    property bool persistentSelectionSetting
    property var spellcheckhighlighter: null
    property var suggestions: ([])
    Component.onCompleted: persistentSelectionSetting = persistentSelectionSetting // break binding

    property var runOnMenuClose 

    parent: Overlay.overlay

    function storeCursorAndSelection() {
        contextMenu.restoredCursorPosition = target.cursorPosition;
        contextMenu.restoredSelectionStart = target.selectionStart;
        contextMenu.restoredSelectionEnd = target.selectionEnd;
    }

    // target is pressed with mouse
    function targetClick(handlerPoint, newTarget, spellcheckhighlighter, mousePosition) {
        if (handlerPoint.pressedButtons === Qt.RightButton) { // only accept just right click
            if (contextMenu.visible) {
                deselectWhenMenuClosed = false; // don't deselect text if menu closed by right click on textfield
                dismiss();
            } else {
                contextMenu.target = newTarget;
                contextMenu.target.persistentSelection = true; // persist selection when menu is opened
                contextMenu.spellcheckhighlighter = spellcheckhighlighter
                contextMenu.suggestions = spellcheckhighlighter.suggestions(mousePosition);
                storeCursorAndSelection();
                popup(contextMenu.target);
                // slightly locate context menu away from mouse so no item is selected when menu is opened
                x += 1
                y += 1
            }
        } else {
            dismiss();
        }
    }

    // context menu keyboard key
    function targetKeyPressed(event, newTarget) {
        if (event.modifiers === Qt.NoModifier && event.key === Qt.Key_Menu) {
            contextMenu.target = newTarget;
            target.persistentSelection = true; // persist selection when menu is opened
            storeCursorAndSelection();
            popup(contextMenu.target);
        }
    }

    readonly property bool targetIsPassword: target !== null && (target.echoMode === TextInput.PasswordEchoOnEdit || target.echoMode === TextInput.Password)

    onAboutToShow: {
        if (Overlay.overlay) {
            let tempZ = 0
            for (let i in Overlay.overlay.visibleChildren) {
                tempZ = Math.max(tempZ, Overlay.overlay.visibleChildren[i].z)
            }
            z = tempZ + 1
        }
    }

    // deal with whether or not text should be deselected
    onClosed: {
        // restore text field's original persistent selection setting
        target.persistentSelection = persistentSelectionSetting
        // deselect text field text if menu is closed not because of a right click on the text field
        if (deselectWhenMenuClosed) {
            target.deselect();
        }
        deselectWhenMenuClosed = true;

        // restore cursor position
        target.forceActiveFocus();
        target.cursorPosition = restoredCursorPosition;
        target.select(restoredSelectionStart, restoredSelectionEnd);

        // run action
        runOnMenuClose();
    }

    onOpened: {
        runOnMenuClose = function() {};
    }

    Instantiator {
        active: target !== null && !target.readOnly && spellcheckhighlighter !== null && spellcheckhighlighter.wordIsMisspelled
        model: suggestions
        delegate: MenuItem {
            text: modelData
            onClicked: {
                deselectWhenMenuClosed = false;
                runOnMenuClose = function() {
                    spellcheckhighlighter.replaceWord(modelData);
                };
            }
        }
        onObjectAdded: contextMenu.insertItem(0, object)
        onObjectRemoved: contextMenu.removeItem(0)
    }

    MenuItem {
        visible: target !== null && !target.readOnly && spellcheckhighlighter !== null && spellcheckhighlighter.wordIsMisspelled && suggestions.length === 0
        action: Action {
            text: spellcheckhighlighter ? i18nc("@action:inmenu", "No suggestions for %1", spellcheckhighlighter.wordUnderMouse) : ""
            enabled: false
        }
    }

    MenuSeparator {
        visible: target !== null && !target.readOnly && spellcheckhighlighter !== null && spellcheckhighlighter.wordIsMisspelled
    }

    MenuItem {
        visible: target !== null && !target.readOnly && spellcheckhighlighter !== null && spellcheckhighlighter.wordIsMisspelled
        action: Action {
            text: i18n("Add to dictionary")
            onTriggered: {
                deselectWhenMenuClosed = false;
                runOnMenuClose = function() {
                    spellcheckhighlighter.addWordToDictionary(spellcheckhighlighter.wordUnderMouse)
                };
            }
        }
    }

    MenuItem {
        visible: target !== null && !target.readOnly && spellcheckhighlighter !== null && spellcheckhighlighter.wordIsMisspelled
        action: Action {
            text: i18n("Ignore")
            onTriggered: {
                deselectWhenMenuClosed = false;
                runOnMenuClose = function() {
                    spellcheckhighlighter.ignoreWord(spellcheckhighlighter.wordUnderMouse)
                };
            }
        }
    }

    MenuSeparator {
        visible: target !== null && !target.readOnly && spellcheckhighlighter !== null && spellcheckhighlighter.wordIsMisspelled
    }

    MenuItem {
        visible: target !== null && !target.readOnly
        action: Action {
            icon.name: "edit-undo-symbolic"
            text: i18nc("@action:inmenu", "Undo")
            shortcut: StandardKey.Undo
        }
        enabled: target !== null && target.canUndo
        onTriggered: {
            deselectWhenMenuClosed = false;
            runOnMenuClose = function() {target.undo()};
        }
    }
    MenuItem {
        visible: target !== null && !target.readOnly
        action: Action {
            icon.name: "edit-redo-symbolic"
            text: i18nc("@action:inmenu", "Redo")
            shortcut: StandardKey.Redo
        }
        enabled: target !== null && target.canRedo
        onTriggered: {
            deselectWhenMenuClosed = false;
            runOnMenuClose = function() {target.redo()};
        }
    }
    MenuSeparator {
        visible: target !== null && !target.readOnly
    }
    MenuItem {
        visible: target !== null && !target.readOnly && !targetIsPassword
        action: Action {
            icon.name: "edit-cut-symbolic"
            text: i18nc("@action:inmenu", "Cut")
            shortcut: StandardKey.Cut
        }
        enabled: target !== null && target.selectedText
        onTriggered: {
            deselectWhenMenuClosed = false;
            runOnMenuClose = function() {target.cut()}
        }
    }
    MenuItem {
        action: Action {
            icon.name: "edit-copy-symbolic"
            text: i18nc("@action:inmenu", "Copy")
            shortcut: StandardKey.Copy
        }
        enabled: target !== null && target.selectedText
        visible: !targetIsPassword
        onTriggered: {
            deselectWhenMenuClosed = false;
            runOnMenuClose = function() {target.copy()}
        }
    }
    MenuItem {
        visible: target !== null && !target.readOnly
        action: Action {
            icon.name: "edit-paste-symbolic"
            text: i18nc("@action:inmenu", "Paste")
            shortcut: StandardKey.Paste
        }
        enabled: target !== null && target.canPaste
        onTriggered: {
            deselectWhenMenuClosed = false;
            runOnMenuClose = function() {target.paste()};
        }
    }
    MenuItem {
        visible: target !== null && !target.readOnly
        action: Action {
            icon.name: "edit-delete-symbolic"
            text: i18nc("@action:inmenu", "Delete")
            shortcut: StandardKey.Delete
        }
        enabled: target !== null && target.selectedText
        onTriggered: {
            deselectWhenMenuClosed = false;
            runOnMenuClose = function() {target.remove(target.selectionStart, target.selectionEnd)};
        }
    }
    MenuSeparator {
        visible: !targetIsPassword
    }
    MenuItem {
        action: Action {
            icon.name: "edit-select-all-symbolic"
            text: i18nc("@action:inmenu", "Select All")
            shortcut: StandardKey.SelectAll
        }
        visible: !targetIsPassword
        onTriggered: {
            deselectWhenMenuClosed = false;
            runOnMenuClose = function() {target.selectAll()};
        }
    }
}
