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
            event.accepted = testHelper.keyHelper.handleKey(event.key, event.modifiers);
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
}
