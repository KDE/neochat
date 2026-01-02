// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtTest

import org.kde.neochat.libneochat

import NeoChatTestUtils

TestCase {
    name: "ChatMarkdownHelperTest"

    TextEdit {
        id: textEdit

        textFormat: TextEdit.RichText
    }

    TextEdit {
        id: textEdit2
    }

    ChatMarkdownHelperTestWrapper {
        id: chatMarkdownHelper

        textItem: textEdit
    }

    SignalSpy {
        id: spyItem
        target: chatMarkdownHelper
        signalName: "textItemChanged"
    }

    SignalSpy {
        id: spyUnhandledFormat
        target: chatMarkdownHelper
        signalName: "unhandledBlockFormat"
    }

    function initTestCase(): void {
        textEdit.forceActiveFocus();
    }

    function cleanup(): void {
        chatMarkdownHelper.clear();
        compare(chatMarkdownHelper.checkText(""), true);
        compare(chatMarkdownHelper.checkFormats([]), true);
        compare(textEdit.cursorPosition, 0);
    }

    function test_item(): void {
        spyItem.clear();
        compare(chatMarkdownHelper.textItem, textEdit);
        chatMarkdownHelper.textItem = textEdit2;
        compare(chatMarkdownHelper.textItem, textEdit2);
        chatMarkdownHelper.textItem = textEdit;
        compare(chatMarkdownHelper.textItem, textEdit);
    }

    function test_textFormat_data() {
        return [
            {tag: "bold", input: "**b** ", outText: ["*", "**", "b", "b*", "b**", "b "], outFormats: [[], [], [RichFormat.Bold], [RichFormat.Bold], [RichFormat.Bold], []], unhandled: 0},
            {tag: "italic", input: "*i* ", outText: ["*", "i", "i*", "i "], outFormats: [[], [RichFormat.Italic], [RichFormat.Italic], []], unhandled: 0},
            {tag: "heading 1", input: "# h", outText: ["#", "# ", "h"], outFormats: [[], [], [RichFormat.Bold, RichFormat.Heading1]], unhandled: 0},
            {tag: "heading 2", input: "## h", outText: ["#", "##", "## ", "h"], outFormats: [[], [], [], [RichFormat.Bold, RichFormat.Heading2]], unhandled: 0},
            {tag: "heading 3", input: "### h", outText: ["#", "##", "###", "### ", "h"], outFormats: [[], [], [], [], [RichFormat.Bold, RichFormat.Heading3]], unhandled: 0},
            {tag: "heading 4", input: "#### h", outText: ["#", "##", "###", "####", "#### ", "h"], outFormats: [[], [], [], [], [], [RichFormat.Bold, RichFormat.Heading4]], unhandled: 0},
            {tag: "heading 5", input: "##### h", outText: ["#", "##", "###", "####", "#####", "##### ", "h"], outFormats: [[], [], [], [], [], [], [RichFormat.Bold, RichFormat.Heading5]], unhandled: 0},
            {tag: "heading 6", input: "###### h", outText: ["#", "##", "###", "####", "#####", "######", "###### ", "h"], outFormats: [[], [], [], [], [], [] ,[], [RichFormat.Bold, RichFormat.Heading6]], unhandled: 0},
            {tag: "quote", input: "> q", outText: [">", "> ", "q"], outFormats: [[], [], []], unhandled: 1},
            {tag: "unorderedlist 1", input: "* l", outText: ["*", "* ", "l"], outFormats: [[], [], [RichFormat.UnorderedList]], unhandled: 0},
            {tag: "unorderedlist 2", input: "- l", outText: ["-", "- ", "l"], outFormats: [[], [], [RichFormat.UnorderedList]], unhandled: 0},
            {tag: "orderedlist 1", input: "1. l", outText: ["1", "1.", "1. ", "l"], outFormats: [[], [], [], [RichFormat.OrderedList]], unhandled: 0},
            {tag: "orderedlist 2", input: "1) l", outText: ["1", "1)", "1) ", "l"], outFormats: [[], [], [], [RichFormat.OrderedList]], unhandled: 0},
            {tag: "inline code", input: "`c` ", outText: ["`", "c", "c`", "c "], outFormats: [[], [RichFormat.InlineCode], [RichFormat.InlineCode], []], unhandled: 0},
            {tag: "code", input: "``` ", outText: ["`", "``", "```", " "], outFormats: [[], [], [], []], unhandled: 1},
            {tag: "strikethrough", input: "~~s~~ ", outText: ["~", "~~", "s", "s~", "s~~", "s "], outFormats: [[], [], [RichFormat.Strikethrough], [RichFormat.Strikethrough], [RichFormat.Strikethrough], []], unhandled: 0},
            {tag: "underline", input: "__u__ ", outText: ["_", "__", "u", "u_", "u__", "u "], outFormats: [[], [], [RichFormat.Underline], [RichFormat.Underline], [RichFormat.Underline], []], unhandled: 0},
            {tag: "multiple closable", input: "***__~~t~~__*** ", outText: ["*", "**", "*", "_", "__", "~", "~~", "t", "t~", "t~~", "t_", "t__", "t*", "t**", "t*", "t "], outFormats: [[], [], [RichFormat.Bold], [RichFormat.Bold, RichFormat.Italic], [RichFormat.Bold, RichFormat.Italic], [RichFormat.Bold, RichFormat.Italic, RichFormat.Underline], [RichFormat.Bold, RichFormat.Italic, RichFormat.Underline], [RichFormat.Bold, RichFormat.Italic, RichFormat.Underline, RichFormat.Strikethrough], [RichFormat.Bold, RichFormat.Italic, RichFormat.Underline, RichFormat.Strikethrough], [RichFormat.Bold, RichFormat.Italic, RichFormat.Underline, RichFormat.Strikethrough], [RichFormat.Bold, RichFormat.Italic, RichFormat.Underline], [RichFormat.Bold, RichFormat.Italic, RichFormat.Underline], [RichFormat.Bold, RichFormat.Italic], [RichFormat.Bold, RichFormat.Italic], [RichFormat.Italic], []], unhandled: 0},
            {tag: "nonclosable closable", input: "* **b** ", outText: ["*", "* ", "*", "**", "b", "b*", "b**", "b "], outFormats: [[], [], [RichFormat.UnorderedList], [RichFormat.UnorderedList], [RichFormat.Bold, RichFormat.UnorderedList], [RichFormat.Bold, RichFormat.UnorderedList], [RichFormat.Bold, RichFormat.UnorderedList], [RichFormat.UnorderedList]], unhandled: 0},
            {tag: "not at line start", input: " 1) ", outText: [" ", " 1", " 1)", " 1) "], outFormats: [[], [], [], []], unhandled: 0},
        ]
    }

    function test_textFormat(data): void {
        spyUnhandledFormat.clear();
        compare(spyUnhandledFormat.count, 0);

        for (let i = 0; i < data.input.length; i++) {
            keyClick(data.input[i]);
            compare(chatMarkdownHelper.checkText(data.outText[i]), true);
            compare(chatMarkdownHelper.checkFormats(data.outFormats[i]), true);
        }

        compare(spyUnhandledFormat.count, data.unhandled);
    }
}
