// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtLocation 5.15
import QtPositioning 5.15

import org.kde.kirigamiaddons.labs.components 1.0 as Components

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

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

    content: Map {
        id: map
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
    }
}
