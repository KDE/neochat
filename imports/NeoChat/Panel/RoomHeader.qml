/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

import org.kde.neochat 1.0
import NeoChat.Effect 1.0
import NeoChat.Component 1.0
import NeoChat.Setting 1.0

Control {
    signal clicked()

    id: header

    background: Rectangle {
        color: MPalette.background

        layer.enabled: true
        layer.effect: ElevationEffect {
            elevation: 2
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 18

        Layout.alignment: Qt.AlignVCenter

        spacing: 12

        Label {
            Layout.fillWidth: true

            text: currentRoom ? currentRoom.displayName : ""
            color: MPalette.foreground
            font.pixelSize: 18
            elide: Text.ElideRight
            wrapMode: Text.NoWrap
        }

        ToolButton {
            Layout.preferredWidth: height
            Layout.fillHeight: true

            contentItem: MaterialIcon {
                icon: "\ue5d4"
                color: MPalette.lighter
            }

            onClicked: header.clicked()
        }
    }
}
