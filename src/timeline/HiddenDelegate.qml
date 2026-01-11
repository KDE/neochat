// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat

TimelineDelegate {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The message author.
     */
    required property NeochatRoomMember author

    required property string eventType

    width: parent?.width
    rightPadding: NeoChatConfig.compactLayout && root.ListView.view.width >= Kirigami.Units.gridUnit * 20 ? Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing

    alwaysFillWidth: NeoChatConfig.compactLayout

    contentItem: QQC2.Control {
        id: contentControl

        topInset: Kirigami.Units.smallSpacing
        topPadding: Kirigami.Units.smallSpacing * 2
        bottomPadding: Kirigami.Units.smallSpacing

        contentItem: RowLayout {
            KirigamiComponents.Avatar {
                Layout.leftMargin: Kirigami.Units.largeSpacing * 1.5
                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small

                name: root.author.displayName
                source: root.author.avatarUrl
                color: root.author.color
                asynchronous: true
            }
            QQC2.Label {
                Layout.fillWidth: true
                text: root.author.displayName + ": " + root.eventType + " " + root.eventId
                color: Kirigami.Theme.disabledTextColor
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                textFormat: Text.PlainText
            }
            Kirigami.Icon {
                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small
                source: "view-hidden"
            }
        }

        TapHandler {
            acceptedButtons: Qt.RightButton
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad | PointerDevice.Stylus
            onTapped: _private.showMessageMenu()
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            acceptedDevices: PointerDevice.TouchScreen
            onLongPressed: _private.showMessageMenu()
        }

        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
            radius: Kirigami.Units.cornerRadius
            border {
                width: contentControl.hovered ? 1 : 0
                color: Kirigami.Theme.highlightColor
            }
        }
    }

    QtObject {
        id: _private

        function showMessageMenu(): void {
            let event = root.Message.room.findEvent(root.eventId);
            RoomManager.viewEventMenu(root.QQC2.Overlay.overlay, event, root.room, "");
        }
    }
}
