// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtTest

import NeoChatTestUtils

TestCase {
    name: "ChatTextItemHelperTest"

    TextEdit {
        id: textEdit
    }

    TextEdit {
        id: textEdit2
    }

    ChatTextItemHelperTestWrapper {
        id: chatTextItemHelper

        textItem: textEdit
    }

    SignalSpy {
        id: spyItem
        target: chatTextItemHelper
        signalName: "textItemChanged"
    }

    SignalSpy {
        id: spyContentsChanged
        target: chatTextItemHelper
        signalName: "contentsChanged"
    }

    SignalSpy {
        id: spyContentsChange
        target: chatTextItemHelper
        signalName: "contentsChange"
    }

    SignalSpy {
        id: spyCursor
        target: chatTextItemHelper
        signalName: "cursorPositionChanged"
    }

    function test_item(): void {
        spyItem.clear();
        compare(chatTextItemHelper.textItem, textEdit);
        compare(spyItem.count, 0);
        chatTextItemHelper.textItem = textEdit2;
        compare(chatTextItemHelper.textItem, textEdit2);
        compare(spyItem.count, 1);
        chatTextItemHelper.textItem = textEdit;
        compare(chatTextItemHelper.textItem, textEdit);
        compare(spyItem.count, 2);
    }

    function test_document(): void {
        // We can't get to the QTextDocument from QML so we have to use a helper function.
        compare(chatTextItemHelper.compareDocuments(textEdit.textDocument), true);
    }

    function test_cursor(): void {
        spyContentsChange.clear();
        spyContentsChanged.clear();
        spyCursor.clear();
        // We can't get to the QTextCursor from QML so we have to use a helper function.
        compare(chatTextItemHelper.compareCursor(textEdit.cursorPosition, textEdit.selectionStart, textEdit.selectionEnd), true);
        compare(textEdit.cursorPosition, chatTextItemHelper.cursorPosition());
        textEdit.insert(0, "test text")
        compare(spyContentsChange.count, 1);
        compare(spyContentsChange.signalArguments[0][0], 0);
        compare(spyContentsChange.signalArguments[0][1], 0);
        compare(spyContentsChange.signalArguments[0][2], 9);
        compare(spyContentsChanged.count, 1);
        compare(spyCursor.count, 1);
        compare(chatTextItemHelper.compareCursor(textEdit.cursorPosition, textEdit.selectionStart, textEdit.selectionEnd), true);
        compare(textEdit.cursorPosition, chatTextItemHelper.cursorPosition());
        textEdit.selectAll();
        compare(spyContentsChanged.count, 1);
        compare(spyCursor.count, 1);
        compare(chatTextItemHelper.compareCursor(textEdit.cursorPosition, textEdit.selectionStart, textEdit.selectionEnd), true);
        compare(textEdit.cursorPosition, chatTextItemHelper.cursorPosition());
        textEdit.clear();
        compare(spyContentsChange.count, 2);
        compare(spyContentsChange.signalArguments[1][0], 0);
        compare(spyContentsChange.signalArguments[1][1], 9);
        compare(spyContentsChange.signalArguments[1][2], 0);
        compare(spyContentsChanged.count, 2);
        compare(spyCursor.count, 2);

    }

    function test_setCursor(): void {
        spyCursor.clear();
        textEdit.insert(0, "test text");
        compare(textEdit.cursorPosition, 9);
        compare(spyCursor.count, 1);
        chatTextItemHelper.setCursorPosition(5);
        compare(textEdit.cursorPosition, 5);
        compare(spyCursor.count, 2);
        chatTextItemHelper.setCursorPosition(1);
        compare(textEdit.cursorPosition, 1);
        compare(spyCursor.count, 3);

        textEdit.cursorVisible = false;
        compare(textEdit.cursorVisible, false);
        chatTextItemHelper.setCursorVisible(true);
        compare(textEdit.cursorVisible, true);
        chatTextItemHelper.setCursorVisible(false);
        compare(textEdit.cursorVisible, false);

        textEdit.clear();
        compare(spyCursor.count, 4);
    }

    function test_forceActiveFocus(): void {
        textEdit2.forceActiveFocus();
        compare(textEdit.activeFocus, false);
        chatTextItemHelper.forceActiveFocus();
        compare(textEdit.activeFocus, true);
    }
}
