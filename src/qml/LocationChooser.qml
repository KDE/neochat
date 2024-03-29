// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation
import QtPositioning

import org.kde.kirigamiaddons.labs.components as Components

import org.kde.kirigami as Kirigami

import org.kde.neochat

Components.AbstractMaximizeComponent {
    id: root

    required property NeoChatRoom room
    property var location

    title: i18n("Choose a Location")

    actions: [
        Kirigami.Action {
            icon.name: "document-send"
            text: i18n("Send this location")
            onTriggered: {
                root.room.sendLocation(root.location.latitude, root.location.longitude, "");
                root.close();
            }
            enabled: !!root.location
        }
    ]

    content: MapView {
        id: mapView
        map.plugin: OsmLocationPlugin.plugin

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.location = mapView.map.toCoordinate(Qt.point(mouseX, mouseY), false);
                mapView.map.addMapItem(mapView.locationMapItem);
            }
        }

        readonly property LocationMapItem locationMapItem: LocationMapItem {
            latitude: root.location.latitude
            longitude: root.location.longitude
            isLive: false
            heading: NaN
            asset: null
            author: null
        }

        Connections {
            target: mapView.map
            function onCopyrightLinkActivated() {
                Qt.openUrlExternally(link);
            }
        }
    }
}
