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

/** Location marker for any of the shared location maps. */
MapQuickItem {
    id: root
    required property real latitude
    required property real longitude

    required property string asset
    required property var author

    required property bool isLive

    anchorPoint.x: sourceItem.width / 2
    anchorPoint.y: sourceItem.height
    coordinate: QtPositioning.coordinate(root.latitude, root.longitude)
    autoFadeIn: false
    sourceItem: Kirigami.Icon {
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
