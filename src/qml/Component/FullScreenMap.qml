// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.15

import org.kde.kirigami 2.15 as Kirigami

ApplicationWindow {
    id: root

    property real latitude: NaN
    property real longitude: NaN
    property string asset
    property var author
    property QtObject liveLocationModel: null

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
        center: root.liveLocationModel ?  QtPositioning.coordinate(root.liveLocationModel.boundingBox.y, root.liveLocationModel.boundingBox.x)
            : QtPositioning.coordinate(root.latitude, root.longitude)
        zoomLevel: 15
        plugin: OsmLocationPlugin.plugin
        LocationMapItem {
            latitude: root.latitude
            longitude: root.longitude
            asset: root.asset
            author: root.author
            isLive: true
            heading: NaN
            visible: !isNaN(root.latitude) && !isNaN(root.longitude)
        }
        MapItemView {
            model: root.liveLocationModel
            delegate: LocationMapItem {}
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
