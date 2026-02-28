// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtTest

import org.kde.neochat.libneochat

import NeoChatTestUtils

TestCase {
    name: "ChatTextItemHelperTest"

    TextEdit {
        id: textEdit
    }

    TextEdit {
        id: textEdit2
    }

    ChatTextItemHelper {
        id: textItemHelper

        textItem: textEdit
    }

    ChatTextItemHelperTestHelper {
        id: testHelper

        textItem: textItemHelper
    }

    SignalSpy {
        id: spyItem
        target: textItemHelper
        signalName: "textItemChanged"
    }

    SignalSpy {
        id: spyContentsChanged
        target: textItemHelper
        signalName: "contentsChanged"
    }

    SignalSpy {
        id: spyContentsChange
        target: textItemHelper
        signalName: "contentsChange"
    }

    SignalSpy {
        id: spyCursor
        target: textItemHelper
        signalName: "cursorPositionChanged"
    }

    function init(): void {
        testHelper.setFixedChars("", "");
        textEdit.clear();
        textEdit2.clear();
        spyItem.clear();
        spyContentsChange.clear();
        spyContentsChanged.clear();
        spyCursor.clear();
    }

    function cleanupTestCase(): void {
        testHelper.textItem = null;
        textItemHelper.textItem = null;
    }

    function test_item(): void {
        compare(textItemHelper.textItem, textEdit);
        compare(spyItem.count, 0);
        textItemHelper.textItem = textEdit2;
        compare(textItemHelper.textItem, textEdit2);
        compare(spyItem.count, 1);
        textItemHelper.textItem = textEdit;
        compare(textItemHelper.textItem, textEdit);
        compare(spyItem.count, 2);
    }

    function test_fixedChars(): void {
        textEdit.forceActiveFocus();
        testHelper.setFixedChars("1", "2");
        compare(textEdit.text, "12");
        compare(textEdit.cursorPosition, 1);
        compare(spyCursor.count, 0);
        keyClick("b");
        compare(textEdit.text, "1b2");
        compare(textEdit.cursorPosition, 2);
        compare(spyCursor.count, 1);
        keyClick(Qt.Key_Left);
        compare(textEdit.text, "1b2");
        compare(textEdit.cursorPosition, 1);
        compare(spyCursor.count, 2);
        keyClick(Qt.Key_Left);
        compare(textEdit.text, "1b2");
        compare(textEdit.cursorPosition, 1);
        compare(spyCursor.count, 3);
        keyClick(Qt.Key_Right);
        compare(textEdit.text, "1b2");
        compare(textEdit.cursorPosition, 2);
        compare(spyCursor.count, 4);
        keyClick(Qt.Key_Right);
        compare(textEdit.text, "1b2");
        compare(textEdit.cursorPosition, 2);
        compare(spyCursor.count, 5);
    }

    function test_longFixedChars(): void {
        textEdit.forceActiveFocus();
        testHelper.setFixedChars("111", "222");
        compare(textEdit.text, "111222");
        compare(textEdit.cursorPosition, 3);
        compare(spyCursor.count, 0);
        keyClick("b");
        compare(textEdit.text, "111b222");
        compare(textEdit.cursorPosition, 4);
        compare(spyCursor.count, 1);
        keyClick(Qt.Key_Left);
        compare(textEdit.text, "111b222");
        compare(textEdit.cursorPosition, 3);
        compare(spyCursor.count, 2);
        keyClick(Qt.Key_Left);
        compare(textEdit.text, "111b222");
        compare(textEdit.cursorPosition, 3);
        compare(spyCursor.count, 3);
        keyClick(Qt.Key_Right);
        compare(textEdit.text, "111b222");
        compare(textEdit.cursorPosition, 4);
        compare(spyCursor.count, 4);
        keyClick(Qt.Key_Right);
        compare(textEdit.text, "111b222");
        compare(textEdit.cursorPosition, 4);
        compare(spyCursor.count, 5);
    }

    function test_document(): void {
        // We can't get to the QTextDocument from QML so we have to use a helper function.
        compare(testHelper.compareDocuments(textEdit.textDocument), true);

        textEdit.insert(0, "test text");
        compare(testHelper.lineCount(), 1);
        textEdit.insert(textEdit.text.length, "\ntest text");
        compare(testHelper.lineCount(), 2);
        textEdit.clear()
        compare(textEdit.text.length, 0);
    }

    function test_takeFirstBlock(): void {
        textEdit.insert(0, "test text");
        compare(testHelper.firstBlockText(), "test text");
        compare(textEdit.text.length, 0);
        textEdit.insert(0, "test text\nmore test text");
        compare(testHelper.firstBlockText(), "test text");
        compare(textEdit.text, "more test text");
        compare(testHelper.firstBlockText(), "more test text");
        compare(textEdit.text, "");
        compare(textEdit.text.length, 0);
    }

    function test_fillFragments(): void {
        textEdit.insert(0, "before fragment\nmid fragment\nafter fragment");
        compare(testHelper.checkFragments("before fragment\nmid fragment", "after fragment", ""), true);
        textEdit.clear();
        textEdit.insert(0, "before fragment\nmid fragment\nafter fragment");
        textEdit.cursorPosition = 16;
        compare(testHelper.checkFragments("before fragment", "mid fragment", "after fragment"), true);
        textEdit.clear();
        textEdit.insert(0, "before fragment\nmid fragment\nafter fragment");
        textEdit.cursorPosition = 29;
        compare(testHelper.checkFragments("before fragment\nmid fragment", "after fragment", ""), true);
        textEdit.clear();
    }

    function test_insertFragment(): void {
        testHelper.insertFragment("test text");
        compare(textEdit.text, "test text");
        compare(textEdit.cursorPosition, 9);
        testHelper.insertFragment("beginning ", 1);
        compare(textEdit.text, "beginning test text");
        compare(textEdit.cursorPosition, 10);
        testHelper.insertFragment(" end", 2);
        compare(textEdit.text, "beginning test text end");
        compare(textEdit.cursorPosition, 23);
        textEdit.clear();

        testHelper.insertFragment("test text", 0, true);
        compare(textEdit.text, "test text");
        compare(textEdit.cursorPosition, 0);
    }

    function test_cursor(): void {
        // We can't get to the QTextCursor from QML so we have to use a helper function.
        compare(testHelper.compareCursor(textEdit.cursorPosition, textEdit.selectionStart, textEdit.selectionEnd), true);
        compare(textEdit.cursorPosition, testHelper.cursorPosition());
        // Check we get the appropriate content and cursor change signals when inserting text.
        textEdit.insert(0, "test text")
        compare(spyContentsChange.count, 1);
        compare(spyContentsChange.signalArguments[0][0], 0);
        compare(spyContentsChange.signalArguments[0][1], 0);
        compare(spyContentsChange.signalArguments[0][2], 9);
        compare(spyContentsChanged.count, 1);
        compare(spyCursor.count, 1);
        compare(spyCursor.signalArguments[0][0], true);
        compare(testHelper.compareCursor(textEdit.cursorPosition, textEdit.selectionStart, textEdit.selectionEnd), true);
        compare(textEdit.cursorPosition, testHelper.cursorPosition());
        // Check we get only get a cursor change signal when moving the cursor.
        textEdit.cursorPosition = 4;
        compare(spyContentsChanged.count, 1);
        compare(spyCursor.count, 2);
        compare(spyCursor.signalArguments[1][0], false);
        textEdit.selectAll();
        compare(spyContentsChanged.count, 1);
        compare(spyCursor.count, 2);
        compare(testHelper.compareCursor(textEdit.cursorPosition, textEdit.selectionStart, textEdit.selectionEnd), true);
        compare(textEdit.cursorPosition, testHelper.cursorPosition());
        // Check we get the appropriate content and cursor change signals when removing text.
        textEdit.clear();
        compare(spyContentsChange.count, 2);
        compare(spyContentsChange.signalArguments[1][0], 0);
        compare(spyContentsChange.signalArguments[1][1], 9);
        compare(spyContentsChange.signalArguments[1][2], 0);
        compare(spyContentsChanged.count, 2);
        compare(spyCursor.count, 3);
        compare(spyCursor.signalArguments[2][0], true);
    }

    function test_setCursor(): void {
        textEdit.insert(0, "test text");
        compare(textEdit.cursorPosition, 9);
        compare(spyCursor.count, 1);
        testHelper.setCursorPosition(5);
        compare(textEdit.cursorPosition, 5);
        compare(spyCursor.count, 2);
        testHelper.setCursorPosition(1);
        compare(textEdit.cursorPosition, 1);
        compare(spyCursor.count, 3);

        textEdit.cursorVisible = false;
        compare(textEdit.cursorVisible, false);
        testHelper.setCursorVisible(true);
        compare(textEdit.cursorVisible, true);
        testHelper.setCursorVisible(false);
        compare(textEdit.cursorVisible, false);
    }

    function test_setCursorFromTextItem(): void {
        textEdit.insert(0, "line 1\nline 2");
        textEdit2.insert(0, "line 1\nline 2");
        testHelper.setCursorFromTextItem(textEdit2, false, 0);
        compare(textEdit.cursorPosition, 7);
        testHelper.setCursorFromTextItem(textEdit2, true, 7);
        compare(textEdit.cursorPosition, 0);
        testHelper.setCursorFromTextItem(textEdit2, false, 1);
        compare(textEdit.cursorPosition, 8);
        testHelper.setCursorFromTextItem(textEdit2, true, 8);
        compare(textEdit.cursorPosition, 1);

        testHelper.setFixedChars("1", "2");
        testHelper.setCursorFromTextItem(textEdit2, false, 0);
        compare(textEdit.cursorPosition, 8);
        testHelper.setCursorFromTextItem(textEdit2, true, 7);
        compare(textEdit.cursorPosition, 1);
    }

    function test_mergeFormat(): void {
        textEdit.insert(0, "lots of text");
        testHelper.setCursorPosition(0);
        testHelper.mergeFormatOnCursor(RichFormat.Bold);
        compare(testHelper.checkFormatsAtCursor([RichFormat.Bold]), true);
        testHelper.mergeFormatOnCursor(RichFormat.Italic);
        compare(testHelper.checkFormatsAtCursor([RichFormat.Bold, RichFormat.Italic]), true);
        testHelper.setCursorPosition(6);
        compare(testHelper.checkFormatsAtCursor([]), true);
        testHelper.mergeFormatOnCursor(RichFormat.Underline);
        compare(testHelper.checkFormatsAtCursor([RichFormat.Underline]), true);
        testHelper.setCursorPosition(9);
        compare(testHelper.checkFormatsAtCursor([]), true);
        testHelper.mergeFormatOnCursor(RichFormat.Strikethrough);
        compare(testHelper.checkFormatsAtCursor([RichFormat.Strikethrough]), true);
        textEdit.clear();

        textEdit.insert(0, "heading");
        testHelper.mergeFormatOnCursor(RichFormat.Heading1);
        compare(testHelper.checkFormatsAtCursor([RichFormat.Bold, RichFormat.Heading1]), true);
        testHelper.mergeFormatOnCursor(RichFormat.Heading2);
        compare(testHelper.checkFormatsAtCursor([RichFormat.Bold, RichFormat.Heading2]), true);
        testHelper.mergeFormatOnCursor(RichFormat.Paragraph);
        compare(testHelper.checkFormatsAtCursor([]), true);
        textEdit.clear();

        textEdit.insert(0, "text");
        testHelper.mergeFormatOnCursor(RichFormat.UnorderedList);
        compare(testHelper.checkFormatsAtCursor([RichFormat.UnorderedList]), true);
        compare(testHelper.markdownText(), "- text");
        testHelper.mergeFormatOnCursor(RichFormat.OrderedList);
        compare(testHelper.checkFormatsAtCursor([RichFormat.OrderedList]), true);
        compare(testHelper.markdownText(), "1.  text");
        textEdit.clear();
    }

    function test_list(): void {
        compare(testHelper.canIndentListMoreAtCursor(), true);
        testHelper.indentListMoreAtCursor();
        compare(testHelper.canIndentListMoreAtCursor(), true);
        testHelper.indentListMoreAtCursor();
        compare(testHelper.canIndentListMoreAtCursor(), true);
        testHelper.indentListMoreAtCursor();
        compare(testHelper.canIndentListMoreAtCursor(), false);

        compare(testHelper.canIndentListLessAtCursor(), true);
        testHelper.indentListLessAtCursor();
        compare(testHelper.canIndentListLessAtCursor(), true);
        testHelper.indentListLessAtCursor();
        compare(testHelper.canIndentListLessAtCursor(), true);
        testHelper.indentListLessAtCursor();
        compare(testHelper.canIndentListLessAtCursor(), false);
    }

    function test_forceActiveFocus(): void {
        textEdit2.forceActiveFocus();
        compare(textEdit.activeFocus, false);
        testHelper.forceActiveFocus();
        compare(textEdit.activeFocus, true);
    }
}
