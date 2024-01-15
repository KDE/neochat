// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation
import QtPositioning

import org.kde.neochat

/**
 * @brief A timeline delegate for a location message.
 *
 * @inherit MessageDelegate
 */
MessageDelegate {
    id: root

    bubbleContent: ColumnLayout {
        LiveLocationsModel {
            id: liveLocationModel
            eventId: root.eventId
            room: root.room
        }
        MapView {
            id: mapView
            Layout.fillWidth: true
            Layout.preferredHeight: root.contentMaxWidth / 16 * 9

            map.center: QtPositioning.coordinate(liveLocationModel.boundingBox.y, liveLocationModel.boundingBox.x)
            map.zoomLevel: 15

            map.plugin: OsmLocationPlugin.plugin

            MapItemView {
                model: liveLocationModel
                delegate: LocationMapItem {}
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: {
                    let map = fullScreenMap.createObject(parent, {liveLocationModel: liveLocationModel});
                    map.open()
                }
                onLongPressed: openMessageContext("")
            }
            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: openMessageContext("")
            }
            Connections {
                target: mapView.map
                function onCopyrightLinkActivated() {
                    Qt.openUrlExternally(link)
                }
            }
        }
        Component {
            id: fullScreenMap
            FullScreenMap {}
        }

        RichLabel {
            textMessage: root.display
            visible: root.display !== ""
        }
    }
}
