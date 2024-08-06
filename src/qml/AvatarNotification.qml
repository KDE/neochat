// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

KirigamiComponents.Avatar {
    id: root

    property int notificationCount

    property bool notificationHighlight

    property bool showNotificationLabel

    QQC2.Label {
        id: notificationCountLabel
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: -Kirigami.Units.smallSpacing
        anchors.rightMargin: -Kirigami.Units.smallSpacing
        z: 1
        width: Math.max(notificationCountTextMetrics.advanceWidth + Kirigami.Units.smallSpacing * 2, height)
        height: Kirigami.Units.iconSizes.smallMedium

        text: root.notificationCount > 0 ? root.notificationCount : ""
        visible: root.showNotificationLabel
        color: Kirigami.Theme.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        background: Rectangle {
            visible: true
            Kirigami.Theme.colorSet: Kirigami.Theme.Button
            Kirigami.Theme.inherit: false
            color: root.notificationHighlight ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.backgroundColor
            radius: height / 2
        }

        TextMetrics {
            id: notificationCountTextMetrics
            text: notificationCountLabel.text
        }
    }
}
