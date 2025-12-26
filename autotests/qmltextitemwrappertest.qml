// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtTest

import NeoChatTestUtils

TestCase {
    name: "QmlTextItemWrapperTest"

    TextEdit {
        id: textEdit
    }

    TextEdit {
        id: textEdit2
    }

    QmlTextItemWrapperTestWrapper {
        id: qmlTextItemWrapper

        textItem: textEdit
    }

    SignalSpy {
        id: spyItem
        target: qmlTextItemWrapper
        signalName: "textItemChanged"
    }

    SignalSpy {
        id: spyContentsChanged
        target: qmlTextItemWrapper
        signalName: "textDocumentContentsChanged"
    }

    SignalSpy {
        id: spyContentsChange
        target: qmlTextItemWrapper
        signalName: "textDocumentContentsChange"
    }

    SignalSpy {
        id: spyCursor
        target: qmlTextItemWrapper
        signalName: "textDocumentCursorPositionChanged"
    }

    function test_item(): void {
        spyItem.clear();
        compare(qmlTextItemWrapper.textItem, textEdit);
        compare(spyItem.count, 0);
        qmlTextItemWrapper.textItem = textEdit2;
        compare(qmlTextItemWrapper.textItem, textEdit2);
        compare(spyItem.count, 1);
        qmlTextItemWrapper.textItem = textEdit;
        compare(qmlTextItemWrapper.textItem, textEdit);
        compare(spyItem.count, 2);
    }

    function test_document(): void {
        // We can't get to the QTextDocument from QML so we have to use a helper function.
        compare(qmlTextItemWrapper.compareDocuments(textEdit.textDocument), true);
    }

    function test_cursor(): void {
        spyContentsChange.clear();
        spyContentsChanged.clear();
        spyCursor.clear();
        // We can't get to the QTextCursor from QML so we have to use a helper function.
        compare(qmlTextItemWrapper.compareCursor(textEdit.cursorPosition, textEdit.selectionStart, textEdit.selectionEnd), true);
        textEdit.insert(0, "test text")
        compare(spyContentsChange.count, 1);
        compare(spyContentsChange.signalArguments[0][0], 0);
        compare(spyContentsChange.signalArguments[0][1], 0);
        compare(spyContentsChange.signalArguments[0][2], 9);
        compare(spyContentsChanged.count, 1);
        compare(spyCursor.count, 1);
        compare(qmlTextItemWrapper.compareCursor(textEdit.cursorPosition, textEdit.selectionStart, textEdit.selectionEnd), true);
        textEdit.selectAll();
        compare(spyContentsChanged.count, 1);
        compare(spyCursor.count, 1);
        compare(qmlTextItemWrapper.compareCursor(textEdit.cursorPosition, textEdit.selectionStart, textEdit.selectionEnd), true);
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
        qmlTextItemWrapper.setCursorPosition(5);
        compare(textEdit.cursorPosition, 5);
        compare(spyCursor.count, 2);
        qmlTextItemWrapper.setCursorPosition(1);
        compare(textEdit.cursorPosition, 1);
        compare(spyCursor.count, 3);

        textEdit.cursorVisible = false;
        compare(textEdit.cursorVisible, false);
        qmlTextItemWrapper.setCursorVisible(true);
        compare(textEdit.cursorVisible, true);
        qmlTextItemWrapper.setCursorVisible(false);
        compare(textEdit.cursorVisible, false);

        textEdit.clear();
        compare(spyCursor.count, 4);
    }

    function test_forceActiveFocus(): void {
        textEdit2.forceActiveFocus();
        compare(textEdit.activeFocus, false);
        qmlTextItemWrapper.forceActiveFocus();
        compare(textEdit.activeFocus, true);
    }
}
