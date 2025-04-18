// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation
import QtPositioning

import org.kde.neochat
import org.kde.kirigami as Kirigami

/**
 * @brief A component to show a location from a message.
 */
ColumnLayout {
    id: root

    /**
     * @brief The message author.
     *
     * A Quotient::RoomMember object.
     *
     * @sa Quotient::RoomMember
     */
    required property var author

    /**
     * @brief The display text of the message.
     */
    required property string display

    /**
     * @brief The latitude of the location marker in the message.
     */
    required property real latitude

    /**
     * @brief The longitude of the location marker in the message.
     */
    required property real longitude

    /**
     * @brief What type of marker the location message is.
     *
     * The main options are m.pin for a general location or m.self for a pin to show
     * a user's location.
     */
    required property string asset

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth

    MapView {
        id: mapView
        Layout.fillWidth: true
        Layout.preferredWidth: root.Message.maxContentWidth
        Layout.preferredHeight: root.Message.maxContentWidth / 16 * 9

        map.center: QtPositioning.coordinate(root.latitude, root.longitude)
        map.zoomLevel: 15

        map.plugin: OsmLocationPlugin.plugin

        readonly property LocationMapItem locationMapItem: LocationMapItem {
            latitude: root.latitude
            longitude: root.longitude
            asset: root.asset
            author: root.author
            isLive: true
            heading: NaN
        }

        Component.onCompleted: map.addMapItem(locationMapItem)

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: {
                let map = fullScreenMap.createObject(parent, {
                    latitude: root.latitude,
                    longitude: root.longitude,
                    asset: root.asset,
                    author: root.author
                });
                map.open();
            }
        }
        TapHandler {
            acceptedDevices: PointerDevice.TouchScreen
            onLongPressed: openMessageContext("")
        }
        TapHandler {
            acceptedButtons: Qt.RightButton
            onTapped: openMessageContext("")
        }
        Connections {
            target: mapView.map
            function onCopyrightLinkActivated(link: string) {
                Qt.openUrlExternally(link);
            }
        }

        Button {
            anchors {
                top: parent.top
                right: parent.right
                margins: Kirigami.Units.smallSpacing
            }

            text: i18nc("@action:button Open the location in an external program", "Open Externally")
            icon.name: "compass"
            onClicked: Qt.openUrlExternally("geo:" + root.latitude + "," + root.longitude)
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
