// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtLocation
import QtPositioning

import org.kde.kirigami as Kirigami

import org.kde.neochat.libneochat

Kirigami.Page {
    id: root

    required property NeoChatRoom room

    title: i18nc("Locations on a map", "Locations")

    padding: 0

    MapView {
        id: mapView
        anchors.fill: parent
        map.plugin: OsmLocationPlugin.plugin
        visible: mapView.map.mapItems.length !== 0

        map.center: {
            let c = LocationHelper.center(LocationHelper.unite(locationsModel.boundingBox, liveLocationsModel.boundingBox));
            return QtPositioning.coordinate(c.y, c.x);
        }
        map.zoomLevel: {
            const zoom = LocationHelper.zoomToFit(LocationHelper.unite(locationsModel.boundingBox, liveLocationsModel.boundingBox), mapView.width, mapView.height)
            return Math.min(Math.max(zoom, map.minimumZoomLevel), map.maximumZoomLevel);
        }

        MapItemView {
            Component.onCompleted: mapView.map.addMapItemView(this)
            anchors.fill: parent

            model: LocationsModel {
                id: locationsModel
                room: root.room
            }
            delegate: LocationMapItem {
                isLive: false
                heading: NaN
            }
        }

        MapItemView {
            Component.onCompleted: mapView.map.addMapItemView(this)
            anchors.fill: parent
            model: LiveLocationsModel {
                id: liveLocationsModel
                room: root.room
            }
            delegate: LocationMapItem {}
        }

        Connections {
            target: mapView.map
            function onCopyrightLinkActivated(link: string): void {
                Qt.openUrlExternally(link);
            }
        }
    }

    Kirigami.PlaceholderMessage {
        text: i18n("There are no locations shared in this room.")
        visible: mapView.map.mapItems.length === 0
        anchors.centerIn: parent
    }
}
