// SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

RowLayout {
    id: row
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
    property var links: model.display.match(/(\bhttps?:\/\/[^\s\<\>\"\']*[^\s\<\>\"\'])/g)
        // don't show previews for room links or user mentions or custom emojis
        .filter(link => !(
            link.includes("https://matrix.to") || (customEmojiLinks && customEmojiLinks.includes(link))
        ))
        // remove ending fullstops and commas
        .map(link => (link.length && [".", ","].includes(link[link.length-1])) ? link.substring(0, link.length-1) : link)
    LinkPreviewer {
        id: lp
        url: links.length > 0 ? links[0] : ""
    }
    visible: lp.loaded && lp.title
    Rectangle {
        Layout.fillHeight: true
        width: Kirigami.Units.smallSpacing
        visible: lp.loaded && lp.title
        color: Kirigami.Theme.highlightColor
    }
    Image {
        visible: lp.imageSource
        Layout.maximumHeight: Kirigami.Units.gridUnit * 5
        Layout.maximumWidth: Kirigami.Units.gridUnit * 5
        source: lp.imageSource.replace("mxc://", "image://mxc/")
        fillMode: Image.PreserveAspectFit
    }
    ColumnLayout {
        id: column
        spacing: Kirigami.Units.smallSpacing
        Kirigami.Heading {
            Layout.maximumWidth: messageDelegate.bubbleMaxWidth
            Layout.fillWidth: true
            level: 4
            wrapMode: Text.Wrap
            textFormat: Text.RichText
            text: "<style>
a {
    text-decoration: none;
}
</style>
            <a href=\"" + links[0] + "\">" + lp.title.replace("&ndash;", "—") + "</a>"
            visible: lp.loaded
            onLinkActivated: RoomManager.openResource(link)
        }
        QQC2.Label {
            text: lp.description
            Layout.maximumWidth: messageDelegate.bubbleMaxWidth
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            visible: lp.loaded && lp.description
        }
    }
}

