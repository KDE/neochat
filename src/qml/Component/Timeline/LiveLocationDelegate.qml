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

    property alias room: liveLocationModel.room

    bubbleContent: ColumnLayout {
        LiveLocationsModel {
            id: liveLocationModel
            eventId: root.eventId
        }
        Map {
            id: map
            Layout.fillWidth: true
            Layout.preferredHeight: root.contentMaxWidth / 16 * 9

            center: QtPositioning.coordinate(liveLocationModel.boundingBox.y, liveLocationModel.boundingBox.x)
            zoomLevel: 15

            plugin: OsmLocationPlugin.plugin
            onCopyrightLinkActivated: Qt.openUrlExternally(link)

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
