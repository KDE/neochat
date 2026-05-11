// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtLocation
import QtPositioning

import org.kde.kirigamiaddons.labs.components as Components

import org.kde.kirigami as Kirigami

import org.kde.neochat.libneochat as LibNeoChat

Components.AbstractMaximizeComponent {
    id: root

    signal locationChosen(latitude: real, longitude: real, asset: string)

    title: i18n("Choose a Location")

    actions: [
        Kirigami.Action {
            icon.name: "map-globe-symbolic"
            text: i18n("Select this location")
            onTriggered: {
                root.locationChosen(mapView.locationMapItem.latitude, mapView.locationMapItem.longitude, "m.pin");
                root.close();
            }
            enabled: mapView.map.mapItems.length > 0
        },
        Kirigami.Action {
            text: i18nc("@action:intoolbar Re-center the map onto the set location", "Re-Center")
            icon.name: "snap-bounding-box-center-symbolic"
            onTriggered: mapView.map.fitViewportToMapItems([mapView.locationMapItem])
            enabled: mapView.map.mapItems.length > 0
        },
        Kirigami.Action {
            text: i18nc("@action:intoolbar Determine the device's location", "Set to current location")
            icon.name: "mark-location-symbolic"
            enabled: positionSource.valid
            onTriggered: positionSource.update()
        }
    ]

    onOpened: forceActiveFocus()

    PositionSource {
        id: positionSource

        active: false

        onPositionChanged: {
            const coord = position.coordinate;
            mapView.locationMapItem.latitude = coord.latitude;
            mapView.locationMapItem.longitude = coord.longitude;

            if (mapView.map.mapItems.length <= 0) {
                mapView.map.addMapItem(mapView.locationMapItem);
            }
            mapView.map.fitViewportToMapItems([mapView.locationMapItem])
        }
    }

    content: MapView {
        id: mapView
        map.plugin: LibNeoChat.OsmLocationPlugin.plugin

        MouseArea {
            anchors.fill: parent
            onClicked: {
                const location = mapView.map.toCoordinate(Qt.point(mouseX, mouseY), false);
                mapView.locationMapItem.latitude = location.latitude;
                mapView.locationMapItem.longitude = location.longitude;
                if (mapView.map.mapItems.length <= 0) {
                    mapView.map.addMapItem(mapView.locationMapItem);
                }
            }
        }

        readonly property LibNeoChat.LocationMapItem locationMapItem: LibNeoChat.LocationMapItem {
            latitude: 0.0
            longitude: 0.0
            isLive: false
            heading: NaN
            asset: ""
            author: null
        }

        Connections {
            target: mapView.map
            function onCopyrightLinkActivated(link: string) {
                Qt.openUrlExternally(link);
            }
        }
    }
}
