// SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.neochat 1.0
import org.kde.kirigami 2.15 as Kirigami

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
    onTextMessageChanged: text = MessageFormatter.format(textMessage, contentLabel.textDocument, contentLabel)

    persistentSelection: true

    // Work around QTBUG 93281
    Component.onCompleted: if (text.includes("<img")) {
        Controller.forceRefreshTextDocument(root.textDocument, root)
    }

    /*
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
<<<<<<< HEAD:src/qml/Component/Timeline/RichLabel.qml
</style>" + textMessage
=======
</style>" + (isEmote ? "* <a href='https://matrix.to/#/" + author.id + "' style='color: " + author.color + "'>" + author.displayName + "</a> " : "") + textMessage + (isEdited ? (" <span style=\"color: " + Kirigami.Theme.disabledTextColor + "\">" + "<span style='font-size: " + Kirigami.Theme.defaultFont.pixelSize +"px'>" + i18n(" (edited)") + "</span>") : "")
    */

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
