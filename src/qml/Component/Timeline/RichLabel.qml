// SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.neochat 1.0
import org.kde.kirigami 2.15 as Kirigami

TextEdit {
    id: contentLabel

    readonly property var isEmoji: /^(<span style='.*'>)?(\u00a9|\u00ae|[\u2000-\u3300]|\ud83c[\ud000-\udfff]|\ud83d[\ud000-\udfff]|\ud83e[\ud000-\udfff])+(<\/span>)?$/
    readonly property var hasSpoiler: /data-mx-spoiler/g

    property bool isEmote: false

    /* Turn all links which aren't already in <a> tags into <a> hyperlinks */
    readonly property var customEmojiLinksRegex: /data-mx-emoticon=""  src="(\bhttps?:\/\/[^\s\<\>\"\']*[^\s\<\>\"\'])/g
    readonly property var customEmojiLinks: {
        let links = [];
        // we need all this because QML JS doesn't support String.matchAll introduced in ECMAScript 2020
        let match = customEmojiLinksRegex.exec(model.display);
        while (match !== null) {
            links.push(match[1])
            match = customEmojiLinksRegex.exec(model.display);
        }
        return links;
    }
    readonly property var linkRegex: /(href=["'])?(\b(https?):\/\/[^\s\<\>\"\'\\]+)/g
    property string textMessage: model.display.includes("http")
        ? model.display.replace(linkRegex, function() {
            if (customEmojiLinks && customEmojiLinks.includes(arguments[0])) {
                return arguments[0];
            }
            if (arguments[1]) {
                return arguments[0];
            }
            const l = arguments[2];
            if ([".", ","].includes(l[l.length-1])) {
                const link = l.substring(0, l.length-1);
                const leftover = l[l.length-1];
                return `<a href="${link}">${link}</a>${leftover}`;
            }
            return `<a href="${l}">${l}</a>`;
        })
        : model.display
    property bool spoilerRevealed: !hasSpoiler.test(textMessage)

    ListView.onReused: Qt.binding(() => !hasSpoiler.test(textMessage))

    persistentSelection: true

    // Work around QTBUG 93281
    Component.onCompleted: if (text.includes("<img")) {
        Controller.forceRefreshTextDocument(contentLabel.textDocument, contentLabel)
    }

    text: "<style>
table {
    width:100%;
    border-width: 1px;
    border-collapse: collapse;
    border-style: solid;
}
table th,
table td {
    border: 1px solid black;
    padding: 3px;
}
pre {
    white-space: pre-wrap
}
a{
    color: " + Kirigami.Theme.linkColor + ";
    text-decoration: none;
}
" + (!spoilerRevealed ? "
[data-mx-spoiler] a {
    color: transparent;
    background: " + Kirigami.Theme.textColor + ";
}
[data-mx-spoiler] {
    color: transparent;
    background: " + Kirigami.Theme.textColor + ";
}
" : "") + "
</style>" + (isEmote ? "* <a href='https://matrix.to/#/" + author.id + "' style='color: " + author.color + "'>" + author.displayName + "</a> " : "") + textMessage + (isEdited ? (" <span style=\"color: " + Kirigami.Theme.disabledTextColor + "\">" + "<span style='font-size: " + Kirigami.Theme.defaultFont.pixelSize +"px'>" + i18n(" (edited)") + "</span>") : "")

    color: Kirigami.Theme.textColor
    selectedTextColor: Kirigami.Theme.highlightedTextColor
    selectionColor: Kirigami.Theme.highlightColor
    font.pointSize: model.reply === undefined && isEmoji.test(model.display) ? Kirigami.Theme.defaultFont.pointSize * 4 : Kirigami.Theme.defaultFont.pointSize
    selectByMouse: !Kirigami.Settings.isMobile
    readOnly: true
    wrapMode: Text.Wrap
    textFormat: Text.RichText

    onLinkActivated: {
        spoilerRevealed = true
        RoomManager.openResource(link)
    }
    onHoveredLinkChanged: if (hoveredLink.length > 0 && hoveredLink !== "1") {
        applicationWindow().hoverLinkIndicator.text = hoveredLink;
    } else {
        applicationWindow().hoverLinkIndicator.text = "";
    }

    HoverHandler {
        cursorShape: (parent.hoveredLink || !spoilerRevealed) ? Qt.PointingHandCursor : Qt.IBeamCursor
    }

    TapHandler {
        enabled: !parent.hoveredLink && !spoilerRevealed
        onTapped: spoilerRevealed = true
    }
}
