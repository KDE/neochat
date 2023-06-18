// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtLocation 5.15
import QtPositioning 5.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

/**
 * @brief A timeline delegate for a location message.
 *
 * @inherit TimelineContainer
 */
TimelineContainer {
    id: root

    property alias room: liveLocationModel.room

    ColumnLayout {
        Layout.maximumWidth: root.contentMaxWidth
        Layout.preferredWidth: root.contentMaxWidth
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
    }
}
