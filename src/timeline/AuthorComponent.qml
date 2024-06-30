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

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    Layout.fillWidth: true
    Layout.maximumWidth: root.maxContentWidth

    implicitHeight: Math.max(nameButton.implicitHeight, timeLabel.implicitHeight)

    QQC2.AbstractButton {
        id: nameButton
        Layout.fillWidth: true
        contentItem: QQC2.Label {
            text: root.author.disambiguatedName
            color: root.author.color
            textFormat: Text.PlainText
            font.weight: Font.Bold
            elide: Text.ElideRight
        }
        Accessible.name: contentItem.text
        onClicked: RoomManager.resolveResource(root.author.uri)
    }
    QQC2.Label {
        id: timeLabel
        text: root.timeString
        horizontalAlignment: Text.AlignRight
        color: Kirigami.Theme.disabledTextColor
        QQC2.ToolTip.visible: timeHoverHandler.hovered
        QQC2.ToolTip.text: root.time.toLocaleString(Qt.locale(), Locale.LongFormat)
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

        HoverHandler {
            id: timeHoverHandler
        }
    }
}
