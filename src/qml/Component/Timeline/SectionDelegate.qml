// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

QQC2.ItemDelegate {
    id: root

    property alias labelText: sectionLabel.text
    property var maxWidth: Number.POSITIVE_INFINITY

    property int colorSet: Kirigami.Theme.Window

    leftPadding: 0
    rightPadding: 0
    topPadding: Kirigami.Units.largeSpacing
    bottomPadding: 0 // Note not 0 by default

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing
        Layout.fillWidth: true

        Kirigami.Heading {
            id: sectionLabel
            level: 4
            color: Kirigami.Theme.disabledTextColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            Layout.fillWidth: true
            Layout.maximumWidth: maxWidth
        }
        Kirigami.Separator {
            Layout.minimumHeight: 2
            Layout.fillWidth: true
            Layout.maximumWidth: maxWidth
        }
    }

    background: Rectangle {
        color: Config.blur ? "transparent" : Kirigami.Theme.backgroundColor
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: root.colorSet
    }
}
