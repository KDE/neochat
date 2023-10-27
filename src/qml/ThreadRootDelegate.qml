// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.labs.components as Components

import org.kde.neochat

Delegates.RoundedItemDelegate {
    id: root

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

    /**
     * @brief The timestamp of the message.
     */
    required property var time

    /**
     * @brief The timestamp of the message as a string.
     */
    required property string timeString

    /**
     * @brief The display text of the message.
     */
    required property string displayText

    /**
     * @brief The ThreadModel for the thread.
     *
     * @sa ThreadModel
     */
    required property var threadModel

    // onClicked: RoomManager.viewEventSource(root.eventId)

    contentItem: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        Components.Avatar {
            Layout.preferredWidth: Kirigami.Units.gridUnit * 2
            Layout.preferredHeight: Kirigami.Units.gridUnit * 2
            Layout.alignment: Qt.AlignTop

            source: root.author?.avatarUrl ?? ""
            name: root.author.displayName
        }

        ColumnLayout {
            Layout.fillWidth: true
            RowLayout {
                Layout.fillWidth: true
                QQC2.Label {
                    Layout.fillWidth: true
                    text: root.author.displayName
                    color: root.author.color
                    textFormat: Text.PlainText
                    font.weight: Font.Bold
                    elide: Text.ElideRight

                    TapHandler {
                        onTapped: RoomManager.visitUser(root.author.object, "mention")
                    }
                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }
                }
                QQC2.Label {
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
            QQC2.Label {
                Layout.fillWidth: true
                text: root.displayText
                elide: Text.ElideRight
                wrapMode: Text.NoWrap
            }
        }
    }
}
