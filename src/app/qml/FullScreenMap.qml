// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning

import org.kde.kirigami as Kirigami

import org.kde.neochat.libneochat

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

    MapView {
        id: mapView
        anchors.fill: parent
        map.center: root.liveLocationModel ? QtPositioning.coordinate(root.liveLocationModel.boundingBox.y, root.liveLocationModel.boundingBox.x) : QtPositioning.coordinate(root.latitude, root.longitude)
        map.zoomLevel: 15
        map.plugin: OsmLocationPlugin.plugin
        LocationMapItem {
            latitude: root.latitude
            longitude: root.longitude
            asset: root.asset
            author: root.author
            isLive: true
            heading: NaN
            visible: !isNaN(root.latitude) && !isNaN(root.longitude)
            Component.onCompleted: mapView.map.addMapItem(this)
        }
        MapItemView {
            model: root.liveLocationModel
            delegate: LocationMapItem {}
            Component.onCompleted: mapView.map.addMapItemView(this)
        }

        Connections {
            target: mapView.map
            function onCopyrightLinkActivated() {
                Qt.openUrlExternally(link);
            }
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
