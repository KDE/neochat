// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

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

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth

    implicitHeight: Math.max(nameButton.implicitHeight, timeLabel.implicitHeight)

    QQC2.Label {
        id: nameButton

        text: root.author.disambiguatedName
        color: root.author.color
        textFormat: Text.PlainText
        font.weight: Font.Bold
        elide: Text.ElideRight

        function openUserMenu(): void {
            const menu = Qt.createComponent("org.kde.neochat", "UserMenu").createObject(root, {
                connection: root.connection,
                window: QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow,
                author: root.author,
            });
            menu.popup(root);
        }

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }

        // tapping to open profile
        TapHandler {
            onTapped: RoomManager.resolveResource(root.author.uri)
        }

        // right-clicking/long-press for context menu
        TapHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad | PointerDevice.Stylus
            acceptedButtons: Qt.RightButton
            onTapped: nameButton.openUserMenu()
        }
        TapHandler {
            acceptedDevices: PointerDevice.TouchScreen
            onTapped: nameButton.openUserMenu()
        }
    }
    Item {
        Layout.fillWidth: true
    }
    QQC2.Label {
        id: timeLabel
        text: root.timeString
        horizontalAlignment: Text.AlignRight
        color: Kirigami.Theme.disabledTextColor
        QQC2.ToolTip.visible: timeHoverHandler.hovered
        QQC2.ToolTip.text: root.time.toLocaleString(Qt.locale(), Locale.ShortFormat)
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

        HoverHandler {
            id: timeHoverHandler
        }
    }
}
