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
        },
        Kirigami.Action {
            text: i18nc("@action:intoolbar Re-center the map onto the set location", "Re-Center")
            icon.name: "snap-bounding-box-center-symbolic"
            onTriggered: mapView.map.fitViewportToMapItems([mapView.locationMapItem])
            enabled: root.location !== undefined
        },
        Kirigami.Action {
            text: i18nc("@action:intoolbar Determine the device's location", "Locate")
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
            mapView.gpsMapItem.latitude = coord.latitude;
            mapView.gpsMapItem.longitude = coord.longitude;

            mapView.map.addMapItem(mapView.gpsMapItem);
            mapView.map.fitViewportToMapItems([mapView.gpsMapItem])
        }
    }

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
            asset: ""
            author: null
        }

        readonly property LocationMapItem gpsMapItem: LocationMapItem {
            latitude: 0.0
            longitude: 0.0
            isLive: true
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
