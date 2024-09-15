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
RowLayout {
    id: root

    /**
     * @brief The matrix ID of the reply event.
     */
    required property var replyEventId

    /**
     * @brief The reply author.
     *
     * A Quotient::RoomMember object.
     *
     * @sa Quotient::RoomMember
     */
    required property var replyAuthor

    /**
     * @brief The model to visualise the content of the message replied to.
     */
    required property var replyContentModel

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    /**
     * @brief The reply has been clicked.
     */
    signal replyClicked(string eventID)

    Layout.fillWidth: true

    spacing: Kirigami.Units.largeSpacing

    Rectangle {
        id: verticalBorder
        Layout.fillHeight: true

        implicitWidth: Kirigami.Units.smallSpacing
        color: root.replyAuthor.color
    }
    ColumnLayout {
        id: contentColumn
        spacing: Kirigami.Units.smallSpacing

        Repeater {
            id: contentRepeater
            model: root.replyContentModel
            delegate: ReplyMessageComponentChooser {
                maxContentWidth: _private.availableContentWidth

                onReplyClicked: root.replyClicked(root.replyEventId)
            }
        }
    }
    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }
    TapHandler {
        acceptedButtons: Qt.LeftButton
        onTapped: root.replyClicked(root.replyEventId)
    }
    QtObject {
        id: _private
        // The space available for the component after taking away the border
        readonly property real availableContentWidth: root.maxContentWidth - verticalBorder.implicitWidth - root.spacing
    }
}
