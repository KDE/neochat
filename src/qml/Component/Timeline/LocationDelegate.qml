// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
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

    ColumnLayout {
        Layout.maximumWidth: root.contentMaxWidth
        Layout.preferredWidth: root.contentMaxWidth
        Map {
            id: map
            Layout.fillWidth: true
            Layout.preferredHeight: root.contentMaxWidth / 16 * 9

            center: QtPositioning.coordinate(root.latitude, root.longitude)
            zoomLevel: 15

            plugin: OsmLocationPlugin.plugin
            onCopyrightLinkActivated: Qt.openUrlExternally(link)

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
        }
        Component {
            id: fullScreenMap
            FullScreenMap { }
        }

        RichLabel {
            textMessage: root.display
            visible: root.display !== ""
        }
    }
}
