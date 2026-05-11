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
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The message author.
     *
     * A Quotient::RoomMember object.
     *
     * @sa Quotient::RoomMember
     */
    required property var author

    /**
     * @brief The Blocks::Block for the delegate.
     */
    required property LocationBlock block

    /**
     * @brief Whether the component should be editable.
     */
    required property bool editable

    /**
     * The maximum height of the image. Can be left undefined most of the times. Passed to MediaSizeHelper::contentMaxHeight.
     */
    property var contentMaxHeight: editable ? Kirigami.Units.gridUnit * 8 : undefined

    required property int index

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth

    MapView {
        id: mapView
        Layout.preferredWidth: root.contentMaxHeight ? (root.contentMaxHeight * (16 / 9)) : root.Message.maxContentWidth
        Layout.preferredHeight: root.contentMaxHeight ? root.contentMaxHeight : (root.Message.maxContentWidth / (16 / 9))

        map.center: QtPositioning.coordinate(root.block.latitude, root.block.longitude)
        map.zoomLevel: 15

        map.plugin: OsmLocationPlugin.plugin

        readonly property LocationMapItem locationMapItem: LocationMapItem {
            latitude: root.block.latitude
            longitude: root.block.longitude
            asset: root.block.asset
            author: root.author
            isLive: true
            heading: NaN
        }

        Component.onCompleted: map.addMapItem(locationMapItem)

        Connections {
            target: mapView.map
            function onCopyrightLinkActivated(link: string) {
                Qt.openUrlExternally(link);
            }
        }

        RowLayout {
            anchors {
                top: parent.top
                right: parent.right
                margins: Kirigami.Units.smallSpacing
            }

            spacing: Kirigami.Units.mediumSpacing

            Button {
                text: i18nc("@action:button Open the location in an external program", "Open Externally")
                icon.name: "open-link-symbolic"
                display: AbstractButton.IconOnly

                onClicked: Qt.openUrlExternally("geo:" + root.block.latitude + "," + root.block.longitude)

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            Button {
                icon.name: "view-fullscreen"
                text: i18nc("@action:button", "Open Fullscreen")
                display: AbstractButton.IconOnly

                onClicked: {
                    let map = fullScreenMap.createObject(parent, {
                        latitude: root.block.latitude,
                        longitude: root.block.longitude,
                        asset: root.block.asset,
                        author: root.author
                    });
                }

                ToolTip.text: text
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
    }
    Component {
        id: fullScreenMap
        FullScreenMap {}
    }
}
