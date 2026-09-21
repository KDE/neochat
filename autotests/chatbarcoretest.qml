// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtTest

import org.kde.neochat.chatbar as ChatBar

import NeoChatTestUtils

TestCase {
    name: "ChatBarCoreTest"

    ChatBarCoreTestHelper {
        id: helper
    }

    ChatBar.ChatBarCore {
        id: core

        room: helper.room
        cache: helper.room.mainCache
        maxAvailableWidth: 500
    }

    function init(): void {
        core.forceActiveFocus();
    }

    function cleanup(): void {
        core.clear();
        verify(core.isEmpty);
    }

    function test_typing_data() {
        return [
            {tag: "basic", input: "test", plainOutput: "test", sendOutput: "test"},
            {tag: "bold markdown", input: "**bold** ", plainOutput: "bold ", sendOutput: "**bold**"}
        ];
    }

    function test_typing(data): void {
        if (!helper.isWindows) {
            for (let i = 0; i < data.input.length; i++) {
                keyClick(data.input[i]);
            }
            compare(helper.cachePlainString(core.cache), data.plainOutput);
            compare(helper.cacheString(core.cache), data.sendOutput);
        }
    }
}
