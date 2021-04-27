// SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.neochat 1.0
import org.kde.kirigami 2.15 as Kirigami

TextEdit {
    id: contentLabel

    Layout.margins: Kirigami.Units.largeSpacing
    Layout.topMargin: 0

    readonly property var isEmoji: /^(\u00a9|\u00ae|[\u2000-\u3300]|\ud83c[\ud000-\udfff]|\ud83d[\ud000-\udfff]|\ud83e[\ud000-\udfff])+$/

    property bool isEmote: false

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

.user-pill{}
</style>" + (isEmote ? "* <a href='https://matrix.to/#/" + author.id + "' style='color: " + author.color + "'>" + author.displayName + "</a> " : "") + model.display + (isEdited ? (" <span style=\"color: " + Kirigami.Theme.disabledTextColor + "\">" + i18n("(edited)") + "</span>") : "")

    color: Kirigami.Theme.textColor
    font.pointSize: isEmoji.test(display) ? Kirigami.Theme.defaultFont.pointSize * 4 : Kirigami.Theme.defaultFont.pointSize
    selectByMouse: !Kirigami.Settings.isMobile
    readOnly: true
    wrapMode: Text.WordWrap
    textFormat: Text.RichText

    onLinkActivated: RoomManager.openResource(link)

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
    }
}
