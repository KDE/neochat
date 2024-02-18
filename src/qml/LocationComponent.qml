// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation
import QtPositioning

import org.kde.neochat

/**
 * @brief A component to show a location from a message.
 */
ColumnLayout {
    id: root

    /**
     * @brief The message author.
     *
     * This should consist of the following:
     *  - id - The matrix ID of the author.
     *  - isLocalUser - Whether the author is the local user.
     *  - avatarSource - The mxc URL for the author's avatar in the current room.
     *  - avatarMediaId - The media ID of the author's avatar.
     *  - avatarUrl - The mxc URL for the author's avatar.
     *  - displayName - The display name of the author.
     *  - display - The name of the author.
     *  - color - The color for the author.
     *  - object - The Quotient::User object for the author.
     *
     * @sa Quotient::User
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

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    Layout.fillWidth: true
    Layout.maximumWidth: root.maxContentWidth

    MapView {
        id: mapView
        Layout.fillWidth: true
        Layout.preferredWidth: root.maxContentWidth
        Layout.preferredHeight: root.maxContentWidth / 16 * 9

        map.center: QtPositioning.coordinate(root.latitude, root.longitude)
        map.zoomLevel: 15

        map.plugin: OsmLocationPlugin.plugin

        LocationMapItem {
            latitude: root.latitude
            longitude: root.longitude
            asset: root.asset
            author: root.author
            isLive: true
            heading: NaN
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: {
                let map = fullScreenMap.createObject(parent, {latitude: root.latitude, longitude: root.longitude, asset: root.asset, author: root.author});
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
        FullScreenMap { }
    }

    TextComponent {
        display: root.display
        visible: root.display !== ""
    }
}
