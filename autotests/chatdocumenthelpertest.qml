// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtTest

import org.kde.neochat.libneochat as LibNeoChat

TestCase {
    name: "ChatDocumentHandlerTest"

    LibNeoChat.ChatDocumentHandler {
        id: documentHandler
    }

    TextEdit {
        id: textEdit
    }

    function test_empty(): void {
        compare(documentHandler.type, LibNeoChat.ChatBarType.None);
        compare(documentHandler.room, null);
        compare(documentHandler.textItem, null);
        compare(documentHandler.atFirstLine, false);
        compare(documentHandler.atLastLine, false);
        compare(documentHandler.bold, false);
        compare(documentHandler.italic, false);
        compare(documentHandler.underline, false);
        compare(documentHandler.strikethrough, false);
        compare(documentHandler.style, 0);
    }
}
