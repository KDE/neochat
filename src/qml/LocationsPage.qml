// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtLocation
import QtPositioning

import org.kde.kirigami as Kirigami
import org.kde.neochat

Kirigami.Page {
    id: root

    required property var room

    title: i18nc("Locations on a map", "Locations")

    padding: 0

    MapView {
        id: mapView
        anchors.fill: parent
        map.plugin: OsmLocationPlugin.plugin

        map.center: {
            let c = LocationHelper.center(LocationHelper.unite(locationsModel.boundingBox, liveLocationsModel.boundingBox));
            return QtPositioning.coordinate(c.y, c.x);
        }
        map.zoomLevel: LocationHelper.zoomToFit(LocationHelper.unite(locationsModel.boundingBox, liveLocationsModel.boundingBox), mapView.width, mapView.height)

        MapItemView {
            model: LocationsModel {
                id: locationsModel
                room: root.room
            }
            delegate: LocationMapItem {
                isLive: true
                heading: NaN
            }
        }

        MapItemView {
            model: LiveLocationsModel {
                id: liveLocationsModel
                room: root.room
            }
            delegate: LocationMapItem {}
        }

        Kirigami.PlaceholderMessage {
            text: i18n("There are no locations shared in this room.")
            visible: mapView.mapItems.length === 0
            anchors.centerIn: parent
        }
        Connections {
            target: mapView.map
            function onCopyrightLinkActivated() {
                Qt.openUrlExternally(link);
            }
        }
    }
}
