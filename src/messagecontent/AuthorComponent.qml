// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.coreaddons 

import org.kde.neochat

RowLayout {
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
     * @brief The timestamp of the message.
     */
    required property var time

    /**
     * @brief The timestamp of the message as a string.
     */
    required property string timeString

    implicitHeight: Math.max(nameButton.implicitHeight, timeLabel.implicitHeight)

    QQC2.Label {
        id: nameButton

        text: root.author.disambiguatedName
        color: root.author.color
        textFormat: Text.PlainText
        font.weight: Font.Bold
        elide: Text.ElideRight
        clip: true // Intentional to limit insane Unicode in display names

        function openUserMenu(): void {
            const menu = Qt.createComponent("org.kde.neochat", "UserMenu").createObject(root, {
                window: QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow,
                author: root.author,
            });
            menu.popup(root.QQC2.Overlay.overlay);
        }

        // tapping to open profile
        TapHandler {
            onTapped: RoomManager.resolveResource(root.author.uri)
        }

        // right-clicking/long-press for context menu
        MouseArea {
            anchors.fill: parent

            acceptedButtons: Qt.RightButton
            cursorShape: Qt.PointingHandCursor

            onPressed: nameButton.openUserMenu()
        }
        TapHandler {
            acceptedDevices: PointerDevice.TouchScreen
            onTapped: nameButton.openUserMenu()
        }
    }
    QQC2.Label {
        id: timeLabel
        text: formattedTime()
        horizontalAlignment: Text.AlignRight
        color: Kirigami.Theme.disabledTextColor
        QQC2.ToolTip.visible: timeHoverHandler.hovered
        QQC2.ToolTip.text: root.time.toLocaleString(Qt.locale(), Locale.LongFormat)
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

        HoverHandler {
            id: timeHoverHandler
        }
        
        
        function formattedTime(): string {
            const days = Math.floor((Date.now() - root.time) / (1000 * 60  * 60 * 24))
            if(days > 0) {
                return Format.formatRelativeDateTime(root.time, Locale.ShortFormat)
            } 
            return root.timeString
        }
    }
}
