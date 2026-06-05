// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtTest

import org.kde.neochat.libneochat

import NeoChatTestUtils

TestCase {
    name: "ChatKeyHelperTest"

    TextEdit {
        id: textEdit

        Keys.onPressed: (event) => {
            event.accepted = testHelper.keyHelper.handleKey(event.key, event.text, event.modifiers);
        }
    }

    ChatTextItemHelper {
        id: textItemHelper

        textItem: textEdit
    }

    ChatKeyHelperTestHelper {
        id: testHelper

        textItem: textItemHelper
    }

    SignalSpy {
        id: spyUp
        target: testHelper.keyHelper
        signalName: "unhandledUp"
    }

    SignalSpy {
        id: spyDown
        target: testHelper.keyHelper
        signalName: "unhandledDown"
    }

    SignalSpy {
        id: spyDelete
        target: testHelper.keyHelper
        signalName: "unhandledDelete"
    }

    SignalSpy {
        id: spyBackSpace
        target: testHelper.keyHelper
        signalName: "unhandledBackspace"
    }

    function init(): void {
        textEdit.clear();
        spyUp.clear();
        spyDown.clear();
        spyDelete.clear();
        spyBackSpace.clear();
        textEdit.forceActiveFocus();
    }

    function cleanupTestCase(): void {
        testHelper.textItem = null;
        textItemHelper.textItem = null;
    }

    function test_upDown(): void {
        textEdit.insert(0, "line 1\nline 2\nline 3")
        textEdit.cursorPosition = 0;
        keyClick(Qt.Key_Up);
        compare(spyUp.count, 1);
        compare(spyDown.count, 0);
        keyClick(Qt.Key_Down);
        compare(spyUp.count, 1);
        compare(spyDown.count, 0);
        keyClick(Qt.Key_Down);
        compare(spyUp.count, 1);
        compare(spyDown.count, 0);
        keyClick(Qt.Key_Down);
        compare(spyUp.count, 1);
        compare(spyDown.count, 1);
    }

    function test_selectPlain(): void {
        textEdit.insert(0, "One Two Three")
        textEdit.cursorPosition = 0;

        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.selectedText, "O");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.selectedText, "On");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.selectedText, "One");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.selectedText, "One ");
        keyClick(Qt.Key_Left);
        compare(textEdit.cursorPosition, 0);
        compare(textEdit.selectedText, "");

        keyClick(Qt.Key_Right, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.selectedText, "One ");
        keyClick(Qt.Key_Right, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.selectedText, "One Two ");
        keyClick(Qt.Key_Right, Qt.ShiftModifier | Qt.ControlModifier);
        keyClick(Qt.Key_Left);
        compare(textEdit.cursorPosition, 0);
        compare(textEdit.selectedText, "");

        textEdit.cursorPosition = 13;
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.selectedText, "e");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.selectedText, "ee");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.selectedText, "ree");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.selectedText, "hree");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.selectedText, "Three");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.selectedText, " Three");
        keyClick(Qt.Key_Right);
        compare(textEdit.cursorPosition, 13);
        compare(textEdit.selectedText, "");

        keyClick(Qt.Key_Left, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.selectedText, "Three");
        keyClick(Qt.Key_Left, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.selectedText, "Two Three");
    }

    function test_moveLink(): void {
        textEdit.insert(0, "One ")
        testHelper.insertLink("https://kde.org", "Link");
        textEdit.insert(9, "Two")

        textEdit.cursorPosition = 2;
        keyClick(Qt.Key_Right);
        compare(textEdit.cursorPosition, 3);
        compare(textEdit.selectionStart, 3);
        compare(textEdit.selectionEnd, 3);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Right);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectionStart, 4);
        compare(textEdit.selectionEnd, 4);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Right);
        compare(textEdit.cursorPosition, 8);
        compare(textEdit.selectionStart, 4);
        compare(textEdit.selectionEnd, 8);
        compare(textEdit.selectedText, "Link");
        keyClick(Qt.Key_Right);
        compare(textEdit.cursorPosition, 8);
        compare(textEdit.selectionStart, 8);
        compare(textEdit.selectionEnd, 8);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Right);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectionStart, 9);
        compare(textEdit.selectionEnd, 9);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Right);
        compare(textEdit.cursorPosition, 10);
        compare(textEdit.selectionStart, 10);
        compare(textEdit.selectionEnd, 10);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Left);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectionStart, 9);
        compare(textEdit.selectionEnd, 9);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Left);
        compare(textEdit.cursorPosition, 8);
        compare(textEdit.selectionStart, 8);
        compare(textEdit.selectionEnd, 8);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Left);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectionStart, 4);
        compare(textEdit.selectionEnd, 8);
        compare(textEdit.selectedText, "Link");
        keyClick(Qt.Key_Left);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectionStart, 4);
        compare(textEdit.selectionEnd, 4);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Left);
        compare(textEdit.cursorPosition, 3);
        compare(textEdit.selectionStart, 3);
        compare(textEdit.selectionEnd, 3);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Left);
        compare(textEdit.cursorPosition, 2);
        compare(textEdit.selectionStart, 2);
        compare(textEdit.selectionEnd, 2);
        compare(textEdit.selectedText, "");
    }

    function test_moveLinkCtrl(): void {
        textEdit.insert(0, "One ")
        testHelper.insertLink("https://kde.org", "Link");
        textEdit.insert(9, "Two")

        textEdit.cursorPosition = 0;
        keyClick(Qt.Key_Right, Qt.ControlModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectionStart, 4);
        compare(textEdit.selectionEnd, 4);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Right, Qt.ControlModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectionStart, 9);
        compare(textEdit.selectionEnd, 9);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Right, Qt.ControlModifier);
        compare(textEdit.cursorPosition, 12);
        compare(textEdit.selectionStart, 12);
        compare(textEdit.selectionEnd, 12);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Left, Qt.ControlModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectionStart, 9);
        compare(textEdit.selectionEnd, 9);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Left, Qt.ControlModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectionStart, 4);
        compare(textEdit.selectionEnd, 4);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Left, Qt.ControlModifier);
        compare(textEdit.cursorPosition, 0);
        compare(textEdit.selectionStart, 0);
        compare(textEdit.selectionEnd, 0);
        compare(textEdit.selectedText, "");

        textEdit.cursorPosition = 12;
        keyClick(Qt.Key_Left, Qt.ControlModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectionStart, 9);
        compare(textEdit.selectionEnd, 9);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Left, Qt.ControlModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectionStart, 4);
        compare(textEdit.selectionEnd, 4);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Left, Qt.ControlModifier);
        compare(textEdit.cursorPosition, 0);
        compare(textEdit.selectionStart, 0);
        compare(textEdit.selectionEnd, 0);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Right, Qt.ControlModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectionStart, 4);
        compare(textEdit.selectionEnd, 4);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Right, Qt.ControlModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectionStart, 9);
        compare(textEdit.selectionEnd, 9);
        compare(textEdit.selectedText, "");
        keyClick(Qt.Key_Right, Qt.ControlModifier);
        compare(textEdit.cursorPosition, 12);
        compare(textEdit.selectionStart, 12);
        compare(textEdit.selectionEnd, 12);
        compare(textEdit.selectedText, "");
    }

    function test_selectLink(): void {
        textEdit.insert(0, "One ")
        testHelper.insertLink("https://kde.org", "Link");
        textEdit.insert(9, "Two")

        textEdit.cursorPosition = 4;
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 8);
        compare(textEdit.selectedText, "Link");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectedText, "Link ");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 10);
        compare(textEdit.selectedText, "Link T");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectedText, "Link ");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 8);
        compare(textEdit.selectedText, "Link");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectedText, "");

        textEdit.cursorPosition = 2;
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 3);
        compare(textEdit.selectedText, "e");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectedText, "e ");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 8);
        compare(textEdit.selectedText, "e Link");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectedText, "e Link ");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 10);
        compare(textEdit.selectedText, "e Link T");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectedText, "e Link ");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 8);
        compare(textEdit.selectedText, "e Link");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectedText, "e ");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 3);
        compare(textEdit.selectedText, "e");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 2);
        compare(textEdit.selectedText, "");

        textEdit.cursorPosition = 8;
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectedText, "Link");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 3);
        compare(textEdit.selectedText, " Link");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 2);
        compare(textEdit.selectedText, "e Link");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 3);
        compare(textEdit.selectedText, " Link");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectedText, "Link");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 8);
        compare(textEdit.selectedText, "");

        textEdit.cursorPosition = 10;
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectedText, "T");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 8);
        compare(textEdit.selectedText, " T");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectedText, "Link T");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 3);
        compare(textEdit.selectedText, " Link T");
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 2);
        compare(textEdit.selectedText, "e Link T");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 3);
        compare(textEdit.selectedText, " Link T");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectedText, "Link T");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 8);
        compare(textEdit.selectedText, " T");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectedText, "T");
        keyClick(Qt.Key_Right, Qt.ShiftModifier);
        compare(textEdit.cursorPosition, 10);
        compare(textEdit.selectedText, "");
    }

    function test_selectLinkCtrl(): void {
        textEdit.insert(0, "One ")
        testHelper.insertLink("https://kde.org", "Link");
        textEdit.insert(9, "Two")

        textEdit.cursorPosition = 0;
        keyClick(Qt.Key_Right, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectionStart, 0);
        compare(textEdit.selectionEnd, 4);
        compare(textEdit.selectedText, "One ");
        keyClick(Qt.Key_Right, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectionStart, 0);
        compare(textEdit.selectionEnd, 9);
        compare(textEdit.selectedText, "One Link ");
        keyClick(Qt.Key_Right, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.cursorPosition, 12);
        compare(textEdit.selectionStart, 0);
        compare(textEdit.selectionEnd, 12);
        compare(textEdit.selectedText, "One Link Two");
        keyClick(Qt.Key_Left, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectionStart, 0);
        compare(textEdit.selectionEnd, 9);
        compare(textEdit.selectedText, "One Link ");
        keyClick(Qt.Key_Left, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectionStart, 0);
        compare(textEdit.selectionEnd, 4);
        compare(textEdit.selectedText, "One ");
        keyClick(Qt.Key_Left, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.cursorPosition, 0);
        compare(textEdit.selectionStart, 0);
        compare(textEdit.selectionEnd, 0);
        compare(textEdit.selectedText, "");

        textEdit.cursorPosition = 12;
        keyClick(Qt.Key_Left, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectionStart, 9);
        compare(textEdit.selectionEnd, 12);
        compare(textEdit.selectedText, "Two");
        keyClick(Qt.Key_Left, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectionStart, 4);
        compare(textEdit.selectionEnd, 12);
        compare(textEdit.selectedText, "Link Two");
        keyClick(Qt.Key_Left, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.cursorPosition, 0);
        compare(textEdit.selectionStart, 0);
        compare(textEdit.selectionEnd, 12);
        compare(textEdit.selectedText, "One Link Two");
        keyClick(Qt.Key_Right, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.cursorPosition, 4);
        compare(textEdit.selectionStart, 4);
        compare(textEdit.selectionEnd, 12);
        compare(textEdit.selectedText, "Link Two");
        keyClick(Qt.Key_Right, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.cursorPosition, 9);
        compare(textEdit.selectionStart, 9);
        compare(textEdit.selectionEnd, 12);
        compare(textEdit.selectedText, "Two");
        keyClick(Qt.Key_Right, Qt.ShiftModifier | Qt.ControlModifier);
        compare(textEdit.cursorPosition, 12);
        compare(textEdit.selectionStart, 12);
        compare(textEdit.selectionEnd, 12);
        compare(textEdit.selectedText, "");
    }
}
