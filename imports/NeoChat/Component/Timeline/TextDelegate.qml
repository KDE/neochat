/**
 * SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2

import org.kde.kirigami 2.4 as Kirigami

TextEdit {
    id: contentLabel

    text: "<style>pre {white-space: pre-wrap} a{color: " + Kirigami.Theme.linkColor + ";} .user-pill{}</style>" + display

    color: Kirigami.Theme.textColor

    readonly property var isEmoji: /^(\u00a9|\u00ae|[\u2000-\u3300]|\ud83c[\ud000-\udfff]|\ud83d[\ud000-\udfff]|\ud83e[\ud000-\udfff])*$/

    font.pointSize: isEmoji.test(message) ? Kirigami.Theme.defaultFont.pointSize * 4 : Kirigami.Theme.defaultFont.pointSize
    selectByMouse: true
    readOnly: true
    wrapMode: Text.WordWrap
    textFormat: Text.RichText

    onLinkActivated: {
        if (link.startsWith("https://matrix.to/")) {
            var result = link.replace(/\?.*/, "").match("https://matrix.to/#/(!.*:.*)/(\\$.*:.*)")
            if (!result || result.length < 3) return
            if (result[1] != currentRoom.id) return
            if (!result[2]) return
            goToEvent(result[2])
        } else {
            Qt.openUrlExternally(link)
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
    }
}
