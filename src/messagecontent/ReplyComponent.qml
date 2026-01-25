// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.coreaddons
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

/**
 * @brief A component to show a message that has been replied to.
 *
 * Similar to the main timeline delegate a reply delegate is chosen based on the type
 * of message being replied to. The main difference is that not all messages can be
 * show in their original form and are instead visualised with a MIME type delegate
 * e.g. Videos.
 */
QQC2.Control {
    id: root

    /**
     * @brief The model to visualise the content of the message replied to.
     */
    required property var replyContentModel

    /**
     * @brief Whether the component should be editable.
     */
    required property bool editable

    /**
     * @brief Extra margin required when anchoring an item on the right.
     *
     * Normally used for scrollbars.
     */
    property int rightAnchorMargin: 0

    Layout.fillWidth: true
    padding: 0

    contentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        Rectangle {
            id: verticalBorder
            Layout.fillHeight: true

            implicitWidth: Kirigami.Units.smallSpacing
            color: root.replyContentModel.author?.color ?? Kirigami.Theme.highlightColor
            radius: Kirigami.Units.cornerRadius
        }
        ColumnLayout {
            id: contentColumn
            spacing: Kirigami.Units.smallSpacing

            Message.maxContentWidth: _private.availableContentWidth

            Repeater {
                id: contentRepeater
                model: root.replyContentModel
                delegate: ReplyMessageComponentChooser {
                    onReplyClicked: RoomManager.goToEvent(root.replyContentModel.eventId)
                }
            }
        }
        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }
        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: RoomManager.goToEvent(root.replyContentModel.eventId)
        }
        QtObject {
            id: _private
            // The space available for the component after taking away the border
            readonly property real availableContentWidth: root.Message.maxContentWidth - verticalBorder.implicitWidth - root.spacing
        }
    }

    QQC2.Button {
        id: cancelButton
        anchors.top: root.top
        anchors.topMargin: Kirigami.Units.smallSpacing
        anchors.right: root.right
        anchors.rightMargin: root.rightAnchorMargin + Kirigami.Units.smallSpacing

        visible: root.editable
        display: QQC2.AbstractButton.IconOnly
        text: i18nc("@action:button", "Cancel reply")
        icon.name: "dialog-close"
        onClicked: root.Message.room.mainCache.replyId = ""
        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
}
