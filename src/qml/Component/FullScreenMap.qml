// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.15

import org.kde.kirigami 2.15 as Kirigami

ApplicationWindow {
    id: root

    required property var content

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
        property string latlong: root.content.geo_uri.split(':')[1]
        property string latitude: latlong.split(',')[0]
        property string longitude: latlong.split(',')[1]
        center: QtPositioning.coordinate(latitude, longitude)
        zoomLevel: 15
        plugin: Plugin {
            name: "osm"
        }
        MapCircle {
            radius: 1500 / map.zoomLevel
            color: Kirigami.Theme.highlightColor
            border.color: Kirigami.Theme.linkColor
            border.width: Kirigami.Units.devicePixelRatio * 2
            smooth: true
            opacity: 0.25
            center: QtPositioning.coordinate(map.latitude, map.longitude)
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
