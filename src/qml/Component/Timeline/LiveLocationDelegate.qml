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

            // center: QtPositioning.coordinate(root.latitude, root.longitude)
            // zoomLevel: 15

            plugin: OsmLocationPlugin.plugin
            onCopyrightLinkActivated: Qt.openUrlExternally(link)

            MapItemView {
                model: liveLocationModel
                delegate: MapQuickItem {
                    anchorPoint.x: sourceItem.width / 2
                    anchorPoint.y: sourceItem.height
                    coordinate: QtPositioning.coordinate(model.latitude, model.longitude)
                    autoFadeIn: false
                    sourceItem: Kirigami.Icon {
                        width: height
                        height: Kirigami.Units.iconSizes.huge
                        source: "gps"
                        isMask: true
                        color: model.isLive ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor

                        Kirigami.Icon {
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: -parent.height / 8
                            visible: model.asset === "m.pin"
                            width: height
                            height: parent.height / 3 + 1
                            source: "pin"
                            isMask: true
                            color: parent.color
                        }
                        Kirigami.Avatar {
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: -parent.height / 8
                            visible: model.asset === "m.self"
                            width: height
                            height: parent.height / 3 + 1
                            name: model.author.displayName
                            source: model.author.avatarSource
                            color: model.author.color
                        }
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onLongPressed: openMessageContext("")
            }
            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: openMessageContext("")
            }
        }
    }
}
