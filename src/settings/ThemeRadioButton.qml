// Copyright 2021 Marco Martin <mart@kde.org>
// Copyright 2018 Furkan Tokac <furkantokac34@gmail.com>
// Copyright 2019 Nate Graham <nate@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

QQC2.RadioButton {
    id: root

    implicitWidth: contentItem.implicitWidth
    implicitHeight: contentItem.implicitHeight

    property alias innerObject: contentLayout.children
    property bool thin

    contentItem: ColumnLayout {
        Kirigami.ShadowedRectangle {
            implicitWidth: implicitHeight * 1.6
            implicitHeight: root.thin ? Kirigami.Units.gridUnit * 5 : Kirigami.Units.gridUnit * 6
            radius: Kirigami.Units.smallSpacing
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View

            shadow.xOffset: 0
            shadow.yOffset: 2
            shadow.size: 10
            shadow.color: Qt.rgba(0, 0, 0, 0.3)

            color: {
                if (root.checked) {
                    return Kirigami.Theme.highlightColor;
                } else if (root.hovered) {
                    // Match appearance of hovered list items
                    return Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.5);
                } else {
                    return Kirigami.Theme.backgroundColor;
                }
            }
            ColumnLayout {
                id: contentLayout
                anchors.fill: parent
                anchors.margins: Kirigami.Units.smallSpacing
                clip: true
            }
        }

        QQC2.Label {
            id: label
            Layout.fillWidth: true
            text: root.text
            horizontalAlignment: Text.AlignHCenter
        }
    }

    indicator: Item {}
    background: Item {}
}
