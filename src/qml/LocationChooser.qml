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
        id: map
        map.plugin: Plugin {
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

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.location = map.toCoordinate(Qt.point(mouseX, mouseY), false)
            }
        }

        MapQuickItem {
            id: point

            visible: root.location
            anchorPoint.x: sourceItem.width / 2
            anchorPoint.y: sourceItem.height * 0.85
            coordinate: root.location
            autoFadeIn: false

            sourceItem: Kirigami.Icon {
                width: height
                height: Kirigami.Units.iconSizes.huge
                source: "gps"
                isMask: true
                color: Kirigami.Theme.highlightColor

                Kirigami.Icon {
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: -parent.height / 8
                    width: height
                    height: parent.height / 3 + 1
                    source: "pin"
                    isMask: true
                    color: Kirigami.Theme.highlightColor
                }
            }
        }
        Connections {
            target: mapView.map
            function onCopyrightLinkActivated() {
                Qt.openUrlExternally(link)
            }
        }
    }
}
