// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import Qt.labs.qmlmodels
import org.kde.kirigami as Kirigami

import org.kde.neochat

TimelineDelegate {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief The timestamp of the event as a neoChatDateTime.
     */
    required property neoChatDateTime dateTime

    property bool isTemporaryHighlighted: false
    onIsTemporaryHighlightedChanged: if (isTemporaryHighlighted) {
        temporaryHighlightTimer.start();
    }

    width: parent?.width
    rightPadding: NeoChatConfig.compactLayout && root.ListView.view.width >= Kirigami.Units.gridUnit * 20 ? Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing

    alwaysFillWidth: NeoChatConfig.compactLayout

    contentItem: QQC2.ItemDelegate {
        padding: Kirigami.Units.largeSpacing * 2
        topPadding: padding
        bottomPadding: Kirigami.Units.largeSpacing
        leftPadding: padding
        rightPadding: padding

        topInset: Kirigami.Units.largeSpacing + Kirigami.Units.mediumSpacing
        bottomInset: Kirigami.Units.mediumSpacing
        rightInset: Kirigami.Units.largeSpacing
        leftInset: 0

        Timer {
            id: temporaryHighlightTimer

            interval: 1500
            onTriggered: root.isTemporaryHighlighted = false
        }

        contentItem: RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: "view-readermode"

                Layout.preferredWidth: Kirigami.Units.iconSizes.sizeForLabels
                Layout.preferredHeight: Kirigami.Units.iconSizes.sizeForLabels
            }

            QQC2.Label {
                text: i18nc("Relative time since the room was last read", "Last read: %1", root.dateTime.relativeDateTime)

                Layout.fillWidth: true
            }

            QQC2.ToolButton {
                text: i18nc("@action:button Mark all messages up to now as read", "Mark as Read")
                icon.name: "checkmark"

                onClicked: root.room.markAllMessagesAsRead()

                Layout.alignment: Qt.AlignRight
            }
        }

        background: Kirigami.ShadowedRectangle {
            id: readMarkerBackground
            color: {
                if (root.isTemporaryHighlighted) {
                    return Kirigami.Theme.positiveBackgroundColor;
                } else {
                    return Kirigami.Theme.backgroundColor;
                }
            }
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            opacity: root.isTemporaryHighlighted ? 1 : 0.6
            radius: Kirigami.Units.cornerRadius
            shadow.size: Kirigami.Units.smallSpacing
            shadow.color: Qt.rgba(0.0, 0.0, 0.0, 0.10)
            border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
            border.width: 1

            Behavior on color {
                ColorAnimation {
                    target: readMarkerBackground
                    duration: Kirigami.Units.veryLongDuration
                    easing.type: Easing.InOutCubic
                }
            }
        }
    }
}
