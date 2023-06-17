// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
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
    required property var content

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


            MapQuickItem {
                id: point

                anchorPoint.x: sourceItem.width / 2
                anchorPoint.y: sourceItem.height
                coordinate: QtPositioning.coordinate(root.latitude, root.longitude)
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
                        visible: root.asset === "m.pin"
                        width: height
                        height: parent.height / 3 + 1
                        source: "pin"
                        isMask: true
                        color: Kirigami.Theme.highlightColor
                    }
                    Kirigami.Avatar {
                        anchors.centerIn: parent
                        anchors.verticalCenterOffset: -parent.height / 8
                        visible: root.asset === "m.self"
                        width: height
                        height: parent.height / 3 + 1
                        name: root.author.displayName
                        source: root.author.avatarSource
                        color: root.author.color
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: {
                    let map = fullScreenMap.createObject(parent, {content: root.content});
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
    }
}
