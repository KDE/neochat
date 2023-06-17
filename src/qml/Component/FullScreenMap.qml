// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.15

import org.kde.kirigami 2.15 as Kirigami

ApplicationWindow {
    id: root

    required property real latitude
    required property real longitude
    required property string asset
    required property var author

    flags: Qt.FramelessWindowHint | Qt.WA_TranslucentBackground
    visibility: Qt.WindowFullScreen

    title: i18n("View Location")

    Shortcut {
        sequence: "Escape"
        onActivated: root.destroy()
    }

    color: Kirigami.Theme.backgroundColor

    background: AbstractButton {
        onClicked: root.destroy()
    }

    Map {
        id: map
        anchors.fill: parent
        center: QtPositioning.coordinate(root.latitude, root.longitude)
        zoomLevel: 15
        plugin: OsmLocationPlugin.plugin
        LocationMapItem {
            latitude: root.latitude
            longitude: root.longitude
            asset: root.asset
            author: root.author
            isLive: true
        }
        onCopyrightLinkActivated: {
            Qt.openUrlExternally(link)
        }
    }

    Button {
        anchors.top: parent.top
        anchors.right: parent.right

        text: i18n("Close")
        icon.name: "dialog-close"
        display: AbstractButton.IconOnly

        width: Kirigami.Units.gridUnit * 2
        height: Kirigami.Units.gridUnit * 2

        onClicked: root.destroy()
    }
}
