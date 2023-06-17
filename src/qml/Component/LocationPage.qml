// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtLocation 5.15
import QtPositioning 5.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.neochat 1.0

Kirigami.Page {
    id: locationsPage

    required property var room

    title: i18nc("Locations on a map", "Locations")

    padding: 0

    Map {
        id: map
        anchors.fill: parent
        plugin: OsmLocationPlugin.plugin

        MapItemView {
            model: LocationsModel {
                room: locationsPage.room
            }
            delegate: LocationMapItem {
                isLive: true
            }
        }

        MapItemView {
            model: LiveLocationsModel {
                room: locationsPage.room
            }
            delegate: LocationMapItem {}
        }
    }
}
