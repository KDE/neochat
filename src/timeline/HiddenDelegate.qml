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
     *
     * This should consist of the following:
     *  - id - The matrix ID of the author.
     *  - isLocalUser - Whether the author is the local user.
     *  - avatarSource - The mxc URL for the author's avatar in the current room.
     *  - avatarMediaId - The media ID of the author's avatar.
     *  - avatarUrl - The mxc URL for the author's avatar.
     *  - displayName - The display name of the author.
     *  - display - The name of the author.
     *  - color - The color for the author.
     *  - object - The Quotient::User object for the author.
     *
     * @sa Quotient::User
     */
    required property var author

    contentItem: QQC2.Control {
        id: contentControl
        contentItem: RowLayout {
            KirigamiComponents.Avatar {
                Layout.leftMargin: Kirigami.Units.largeSpacing * 1.5
                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small

                name: root.author.displayName
                source: root.author.avatarSource
                color: root.author.color
            }
            QQC2.Label {
                Layout.fillWidth: true
                text: root.author.displayName + " : " + root.eventId
                color: Kirigami.Theme.disabledTextColor
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            Kirigami.Icon {
                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small
                source: "view-hidden"
            }
        }

        TapHandler {
            acceptedButtons: Qt.RightButton
            onTapped: _private.showMessageMenu()
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
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
        function showMessageMenu() {
            RoomManager.viewEventMenu(root.eventId, root.room, "");
        }
    }
}
