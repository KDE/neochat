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
        plugin: Plugin {
            name: "osm"
            PluginParameter {
                name: "osm.useragent"
                value: Application.name + "/" + Application.version + " (kde-devel@kde.org)"
            }
            PluginParameter {
                name: "osm.mapping.providersrepository.address"
                value: "https://autoconfig.kde.org/qtlocation/"
            }
        }

        MapItemView {
            model: LocationsModel {
                room: locationsPage.room
            }
            delegate: MapQuickItem {
                id: point

                required property var longitude
                required property var latitude
                required property string text
                anchorPoint.x: icon.width / 2
                anchorPoint.y: icon.height / 2
                coordinate: QtPositioning.coordinate(point.latitude, point.longitude)
                autoFadeIn: false
                sourceItem: Kirigami.Icon {
                    id: icon
                    width: height
                    height: Kirigami.Units.iconSizes.medium
                    source: "flag-blue"
                }
            }
        }
    }
}
