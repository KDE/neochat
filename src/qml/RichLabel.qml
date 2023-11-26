// SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Layouts

import org.kde.neochat
import org.kde.kirigami as Kirigami

/**
 * @brief A component to show the rich display text of text message.
 */
TextEdit {
    id: root

    /**
     * @brief The rich text message to display.
     */
    property string textMessage

    /**
     * @brief Whether this message is replying to another.
     */
    property bool isReply

    /**
     * @brief Regex for detecting a message with a single emoji.
     */
    readonly property var isEmojiRegex: /^(<span style='.*'>)?(\u00a9|\u00ae|[\u20D0-\u2fff]|[\u3190-\u3300]|\ud83c[\ud000-\udfff]|\ud83d[\ud000-\udfff]|\ud83e[\ud000-\udfff])+(<\/span>)?$/

    /**
     * @brief Whether the message is an emoji
     */
    readonly property var isEmoji: isEmojiRegex.test(textMessage)

    /**
     * @brief Regex for detecting a message with a spoiler.
     */
    readonly property var hasSpoiler: /data-mx-spoiler/g

    /**
     * @brief Whether a spoiler should be revealed.
     */
    property bool spoilerRevealed: !hasSpoiler.test(textMessage)

    ListView.onReused: Qt.binding(() => !hasSpoiler.test(textMessage))

    persistentSelection: true

    // Work around QTBUG 93281
    Component.onCompleted: if (text.includes("<img")) {
        Controller.forceRefreshTextDocument(root.textDocument, root)
    }

    text: "<style>
table {
    width:100%;
    border-width: 1px;
    border-collapse: collapse;
    border-style: solid;
}
code {
    background-color:" + Kirigami.Theme.alternateBackgroundColor + ";
}
table th,
table td {
    border: 1px solid black;
    padding: 3px;
}
blockquote {
    margin: 0;
}
blockquote table {
    width: 100%;
    border-width: 0;
    background-color:" + Kirigami.Theme.alternateBackgroundColor + ";
}
blockquote td {
    width: 100%;
    padding: " + Kirigami.Units.largeSpacing + ";
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
</style>" + textMessage

    color: Kirigami.Theme.textColor
    selectedTextColor: Kirigami.Theme.highlightedTextColor
    selectionColor: Kirigami.Theme.highlightColor
    font {
        pointSize: !root.isReply && root.isEmoji ? Kirigami.Theme.defaultFont.pointSize * 4 : Kirigami.Theme.defaultFont.pointSize
        family: root.isEmoji ? 'emoji' : Kirigami.Theme.defaultFont.family
    }
    selectByMouse: !Kirigami.Settings.isMobile
    readOnly: true
    wrapMode: Text.Wrap
    textFormat: Text.RichText

    onLinkActivated: link => {
        spoilerRevealed = true
        RoomManager.resolveResource(link, "join")
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
