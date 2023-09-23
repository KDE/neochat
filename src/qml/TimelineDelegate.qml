// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.config

/**
 * @brief The base Item for all delegates in the timeline.
 *
 * This component handles the placing of the main content for a delegate in the
 * timeline. The component is designed for all delegates, positioning them in the
 * timeline with variable padding depending on the window width.
 *
 * This component also supports always setting the delegate to fill the available
 * width in the timeline, e.g. in compact mode.
 */
Item {
    id: root

    /**
     * @brief The Item representing the delegate's main content.
     */
    property Item contentItem

    /**
     * @brief Whether the delegate should always stretch to the maximum available width.
     */
    property bool alwaysMaxWidth: false

    /**
     * @brief The padding to the left of the content.
     */
    property real leftPadding: Kirigami.Units.largeSpacing

    /**
     * @brief The padding to the right of the content.
     */
    property real rightPadding: Config.compactLayout && root.ListView.view.width >= Kirigami.Units.gridUnit * 20 ? Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing

    width: parent?.width
    implicitHeight: contentItemParent.implicitHeight

    Item {
        id: contentItemParent
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: state === "alignLeft" ? Kirigami.Units.largeSpacing : 0

        state: Config.compactLayout || root.alwaysMaxWidth ? "alignLeft" : "alignCenter"
        // Align left when in compact mode and center when using bubbles
        states: [
            State {
                name: "alignLeft"
                AnchorChanges {
                    target: contentItemParent
                    anchors.horizontalCenter: undefined
                    anchors.left: parent ? parent.left : undefined
                }
            },
            State {
                name: "alignCenter"
                AnchorChanges {
                    target: contentItemParent
                    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
                    anchors.left: undefined
                }
            }
        ]

        width: (Config.compactLayout || root.alwaysMaxWidth ? root.width : delegateSizeHelper.currentWidth) - root.leftPadding - root.rightPadding
        implicitHeight: root.contentItem?.implicitHeight ?? 0
    }

    DelegateSizeHelper {
        id: delegateSizeHelper
        startBreakpoint: Kirigami.Units.gridUnit * 46
        endBreakpoint: Kirigami.Units.gridUnit * 66
        startPercentWidth: 100
        endPercentWidth: 85
        maxWidth: Kirigami.Units.gridUnit * 60

        parentWidth: root.width
    }

    onContentItemChanged: {
        if (!contentItem) {
            return;
        }

        contentItem.parent = contentItemParent;
        contentItem.anchors.fill = contentItem.parent;
    }
}
