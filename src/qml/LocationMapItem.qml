// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation
import QtPositioning

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

/** Location marker for any of the shared location maps. */
MapQuickItem {
    id: root
    required property real latitude
    required property real longitude

    required property string asset
    required property var author

    required property bool isLive

    required property real heading

    anchorPoint.x: sourceItem.width / 2
    anchorPoint.y: sourceItem.height
    coordinate: QtPositioning.coordinate(root.latitude, root.longitude)
    autoFadeIn: false
    sourceItem: Kirigami.Icon {
        id: mainIcon
        width: height
        height: Kirigami.Units.iconSizes.huge
        source: "gps"
        isMask: true
        color: root.isLive ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor

        Kirigami.Icon {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -parent.height / 8
            visible: root.asset === "m.pin"
            width: height
            height: parent.height / 3 + 1
            source: "pin"
            isMask: true
            color: parent.color
        }
        KirigamiComponents.Avatar {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -parent.height / 8
            visible: root.asset === "m.self"
            width: height
            height: parent.height / 3 + 1
            name: root.author.displayName
            source: root.author.avatarSource
            color: root.author.color
        }

        Kirigami.Icon {
            id: headingIcon
            source: "go-up-symbolic"
            color: parent.color
            visible: !isNaN(root.heading) && root.isLive
            anchors.bottom: mainIcon.top
            anchors.horizontalCenter: mainIcon.horizontalCenter
            transform: Rotation {
                origin.x: headingIcon.width/2
                origin.y: headingIcon.height + mainIcon.height/2
                angle: root.heading
            }
        }
    }
}
