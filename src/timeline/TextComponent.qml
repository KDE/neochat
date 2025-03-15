// SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component to show rich text from a message.
 */
TextEdit {
    id: root

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The message author.
     *
     * A Quotient::RoomMember object.
     *
     * @sa Quotient::RoomMember
     */
    required property NeochatRoomMember author

    /**
     * @brief The display text of the message.
     */
    required property string display

    /**
     * @brief Whether this message is replying to another.
     */
    property bool isReply: false

    /**
     * @brief Regex for detecting a message with a spoiler.
     */
    readonly property var hasSpoiler: /data-mx-spoiler/g

    /**
     * @brief Whether a spoiler should be revealed.
     */
    property bool spoilerRevealed: !hasSpoiler.test(display)

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.maximumWidth: Message.maxContentWidth

    ListView.onReused: Qt.binding(() => !hasSpoiler.test(display))

    persistentSelection: true

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
</style>" + display

    color: Kirigami.Theme.textColor
    selectedTextColor: Kirigami.Theme.highlightedTextColor
    selectionColor: Kirigami.Theme.highlightColor
    font {
        pointSize: !root.isReply && QmlUtils.isEmoji(display) ? Kirigami.Theme.defaultFont.pointSize * 4 : Kirigami.Theme.defaultFont.pointSize
        family: QmlUtils.isEmoji(display) ? 'emoji' : Kirigami.Theme.defaultFont.family
    }
    selectByMouse: !Kirigami.Settings.isMobile
    readOnly: true
    wrapMode: Text.Wrap
    textFormat: Text.RichText

    onLinkActivated: link => {
        spoilerRevealed = true;
        RoomManager.resolveResource(link, "join");
    }
    onHoveredLinkChanged: if (hoveredLink.length > 0 && hoveredLink !== "1") {
        applicationWindow().hoverLinkIndicator.text = hoveredLink;
    } else {
        applicationWindow().hoverLinkIndicator.text = "";
    }

    HoverHandler {
        cursorShape: (root.hoveredLink || !spoilerRevealed) ? Qt.PointingHandCursor : Qt.IBeamCursor
    }
    TapHandler {
        enabled: !root.hoveredLink && !spoilerRevealed
        onTapped: spoilerRevealed = true
    }
    TapHandler {
        enabled: !root.hoveredLink
        acceptedButtons: Qt.LeftButton
        acceptedDevices: PointerDevice.TouchScreen
        onLongPressed: RoomManager.viewEventMenu(root.eventId, root.Message.room, root.author, root.Message.selectedText, root.Message.hoveredLink);
    }
    TapHandler {
        acceptedButtons: Qt.RightButton
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad | PointerDevice.Stylus
        gesturePolicy: TapHandler.WithinBounds
        onTapped: RoomManager.viewEventMenu(root.eventId, root.Message.room, root.author, root.Message.selectedText, root.Message.hoveredLink);
    }
}
