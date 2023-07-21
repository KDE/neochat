// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents

import org.kde.neochat 1.0

RowLayout {
    id: root
    property var name
    property alias avatar: stateAvatar.source
    property var color
    property alias text: label.text

    signal avatarClicked()
    signal linkClicked(string link)

    implicitHeight: Math.max(label.contentHeight, stateAvatar.implicitHeight)

    KirigamiComponents.Avatar {
        id: stateAvatar

        Layout.preferredWidth: Kirigami.Units.iconSizes.small
        Layout.preferredHeight: Kirigami.Units.iconSizes.small

        name: root.name
        color: root.color

        Rectangle {
            radius: height
            height: 4
            width: 4
            color: root.color
            anchors.centerIn: parent
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: avatarClicked()
        }
    }

    QQC2.Label {
        id: label
        Layout.alignment: Qt.AlignVCenter
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        textFormat: Text.RichText
        onLinkActivated: linkClicked(link)
    }
}
