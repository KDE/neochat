import QtQuick 2.9
import QtQuick.Controls 2.2

import Spectral 0.1

import "qrc:/js/md.js" as Markdown

TextArea {
    property real progress: 0

    wrapMode: Text.Wrap
    placeholderText: "Send a Message"
    leftPadding: 16
    topPadding: 0
    bottomPadding: 0
    selectByMouse: true
    verticalAlignment: TextEdit.AlignVCenter

    text: currentRoom ? currentRoom.cachedInput : ""

    background: Item {
    }

    Timer {
        id: timeoutTimer

        repeat: false
        interval: 2000
        onTriggered: {
            repeatTimer.stop()
            currentRoom.sendTypingNotification(false)
        }
    }

    Timer {
        id: repeatTimer

        repeat: true
        interval: 5000
        triggeredOnStart: true
        onTriggered: currentRoom.sendTypingNotification(true)
    }

    ToolTip.visible: currentRoom
                     && currentRoom.hasUsersTyping
    ToolTip.text: currentRoom ? currentRoom.usersTyping : ""

    Keys.onReturnPressed: {
        if (event.modifiers & Qt.ShiftModifier) {
            insert(cursorPosition, "\n")
        } else {
            postMessage(text)
            text = ""
        }
    }

    onTextChanged: {
        timeoutTimer.restart()
        repeatTimer.start()
        currentRoom.cachedInput = text
    }

    function postMessage(text) {
        if (text.trim().length === 0) { return }
        if(!currentRoom) { return }

        var PREFIX_ME = '/me '
        var PREFIX_NOTICE = '/notice '
        var PREFIX_RAINBOW = '/rainbow '
        var PREFIX_HTML = '/html '
        var PREFIX_MARKDOWN = '/md '

        var replyRe = new RegExp("^> <(.*)><(.*)> (.*)\n\n(.*)")
        if (text.match(replyRe)) {
            var matches = text.match(replyRe)
            currentRoom.sendReply(matches[1], matches[2], matches[3], matches[4])
            return
        }

        if (text.indexOf(PREFIX_ME) === 0) {
            text = text.substr(PREFIX_ME.length)
            currentRoom.postMessage(text, RoomMessageEvent.Emote)
            return
        }
        if (text.indexOf(PREFIX_NOTICE) === 0) {
            text = text.substr(PREFIX_NOTICE.length)
            currentRoom.postMessage(text, RoomMessageEvent.Notice)
            return
        }
        if (text.indexOf(PREFIX_RAINBOW) === 0) {
            text = text.substr(PREFIX_RAINBOW.length)

            var parsedText = ""
            var rainbowColor = ["#ff2b00", "#ff5500", "#ff8000", "#ffaa00", "#ffd500", "#ffff00", "#d4ff00", "#aaff00", "#80ff00", "#55ff00", "#2bff00", "#00ff00", "#00ff2b", "#00ff55", "#00ff80", "#00ffaa", "#00ffd5", "#00ffff", "#00d4ff", "#00aaff", "#007fff", "#0055ff", "#002bff", "#0000ff", "#2a00ff", "#5500ff", "#7f00ff", "#aa00ff", "#d400ff", "#ff00ff", "#ff00d4", "#ff00aa", "#ff0080", "#ff0055", "#ff002b", "#ff0000"]
            for (var i = 0; i < text.length; i++) {
                parsedText = parsedText + "<font color='" + rainbowColor[i % rainbowColor.length] + "'>" + text.charAt(i) + "</font>"
            }
            currentRoom.postHtmlMessage(text, parsedText, RoomMessageEvent.Text)
            return
        }
        if (text.indexOf(PREFIX_HTML) === 0) {
            text = text.substr(PREFIX_HTML.length)
            var re = new RegExp("<.*?>")
            var plainText = text.replace(re, "")
            currentRoom.postHtmlMessage(plainText, text, RoomMessageEvent.Text)
            return
        }
        if (text.indexOf(PREFIX_MARKDOWN) === 0) {
            text = text.substr(PREFIX_MARKDOWN.length)
            var parsedText = Markdown.markdown_parser(text)
            currentRoom.postHtmlMessage(text, parsedText, RoomMessageEvent.Text)
            return
        }

        currentRoom.postPlainText(text)
    }
}
