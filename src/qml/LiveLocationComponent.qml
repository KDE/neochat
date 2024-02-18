// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation
import QtPositioning

import org.kde.neochat

/**
 * @brief A component to show a live location from a message.
 */
ColumnLayout {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The display text of the message.
     */
    required property string display

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    Layout.fillWidth: true
    Layout.maximumWidth: root.maxContentWidth

    LiveLocationsModel {
        id: liveLocationModel
        eventId: root.eventId
        room: root.room
    }
    MapView {
        id: mapView
        Layout.fillWidth: true
        Layout.preferredWidth: root.maxContentWidth
        Layout.preferredHeight: root.maxContentWidth / 16 * 9

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

    TextComponent {
        display: root.display
        visible: root.display !== ""
    }
}
