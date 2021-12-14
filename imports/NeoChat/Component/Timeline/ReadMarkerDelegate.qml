// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import Qt.labs.qmlmodels 1.0
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

QQC2.ItemDelegate {
    padding: Kirigami.Units.largeSpacing
    topInset: Kirigami.Units.largeSpacing
    topPadding: Kirigami.Units.largeSpacing * 2
    width: ListView.view.width - Kirigami.Units.gridUnit
    x: Kirigami.Units.gridUnit / 2
    contentItem: QQC2.Label {
        text: i18nc("Relative time since the room was last read", "Last read: %1", time)
    }

    background: Kirigami.ShadowedRectangle {
        color: Kirigami.Theme.backgroundColor
        opacity: 0.6
        radius: Kirigami.Units.smallSpacing
        shadow.size: Kirigami.Units.smallSpacing
        shadow.color: Qt.rgba(0.0, 0.0, 0.0, 0.10)
        border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
        border.width: 1
    }

    Timer {
        id: makeMeDisapearTimer
        interval: Kirigami.Units.humanMoment * 2
        onTriggered: if (QQC2.ApplicationWindow.window.visibility !== QQC2.ApplicationWindow.Hidden) {
            currentRoom.markAllMessagesAsRead();
        }
    }

    ListView.onPooled: makeMeDisapearTimer.stop()

    ListView.onAdd: {
        const view = ListView.view;
        if (view.atYEnd) {
            makeMeDisapearTimer.start()
        }
    }

    // When the read marker is visible and we are at the end of the list,
    // start the makeMeDisapearTimer
    Connections {
        target: ListView.view
        function onAtYEndChanged() {
            makeMeDisapearTimer.start();
        }
    }


    ListView.onRemove: {
        const view = ListView.view;

        if (view.atYEnd) {
            // easy case just mark everything as read
            if (QQC2.ApplicationWindow.window.visibility !== QQC2.ApplicationWindow.Hidden) {
                currentRoom.markAllMessagesAsRead();
            }
            return;
        }

        // mark the last visible index
        const lastVisibleIdx = lastVisibleIndex();

        if (lastVisibleIdx < index) {
            currentRoom.readMarkerEventId = sortedMessageEventModel.data(sortedMessageEventModel.index(lastVisibleIdx, 0), MessageEventModel.EventIdRole)
        }
    }
}
