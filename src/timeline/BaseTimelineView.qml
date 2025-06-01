// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick

import org.kde.neochat.libneochat as LibNeoChat

ListView {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property LibNeoChat.NeoChatRoom room

    /**
     * @brief Get the row number for the given event ID.
     */
    function eventToIndex(eventID) {
        const index = root.timelineModel.timelineMessageModel.eventIdToRow(eventID);
        if (index === -1)
            return -1;
        return root.messageFilterModel.mapFromSource(root.timelineModel.index(index, 0)).row;
    }

    /**
     * @brief Shift the view to the given event ID.
     */
    function goToEvent(eventID) {
        const index = eventToIndex(eventID);
        if (index == -1) {
            messageListView.positionViewAtEnd();
            return;
        }
        messageListView.positionViewAtIndex(index, ListView.Center);
        itemAtIndex(index).isTemporaryHighlighted = true;
    }

    spacing: 0
    verticalLayoutDirection: ListView.BottomToTop
    clip: true

    Connections {
        target: root.model
        function onModelResetComplete() {
            root.positionViewAtBeginning();
        }
    }

    delegate: EventDelegate {
        room: root.room
    }

    KirigamiComponents.FloatingButton {
        id: goReadMarkerFab

        anchors {
            right: parent.right
            top: parent.top
            topMargin: Kirigami.Units.largeSpacing
            rightMargin: Kirigami.Units.largeSpacing
        }

        implicitWidth: Kirigami.Settings.hasTransientTouchInput ? Kirigami.Units.gridUnit * 3 : Kirigami.Units.gridUnit * 2
        implicitHeight: Kirigami.Settings.hasTransientTouchInput ? Kirigami.Units.gridUnit * 3 : Kirigami.Units.gridUnit * 2

        padding: Kirigami.Units.largeSpacing

        z: 2
        visible: (!root.currentRoom?.partiallyReadStats.empty())

        text: root.currentRoom.readMarkerLoaded ? i18n("Jump to first unread message") : i18n("Jump to oldest loaded message")
        action: Kirigami.Action {
            onTriggered: {
                if (!Kirigami.Settings.isMobile) {
                    root.focusChatBar();
                }
                goReadMarkerFab.textChanged()
                messageListView.goToEvent(root.currentRoom.lastFullyReadEventId);
            }
            icon.name: "go-up"
            shortcut: "Shift+PgUp"
        }

        QQC2.ToolTip {
            text: goReadMarkerFab.text
            delay: Kirigami.Units.toolTipDelay
            visible: goReadMarkerFab.hovered
        }
    }
    KirigamiComponents.FloatingButton {
        id: goMarkAsReadFab
        anchors {
            right: parent.right
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
            rightMargin: Kirigami.Units.largeSpacing
        }
        implicitWidth: Kirigami.Settings.hasTransientTouchInput ? Kirigami.Units.gridUnit * 3 : Kirigami.Units.gridUnit * 2
        implicitHeight: Kirigami.Settings.hasTransientTouchInput ? Kirigami.Units.gridUnit * 3 : Kirigami.Units.gridUnit * 2

        padding: Kirigami.Units.largeSpacing

        z: 2
        visible: !messageListView.atYEnd

        text: i18n("Jump to latest message")
        action: Kirigami.Action {
            onTriggered: {
                root.positionViewAtBeginning();
                root.currentRoom.markAllMessagesAsRead();
            }
            icon.name: "go-down"
            shortcut: "Shift+PgDown"
        }

        QQC2.ToolTip {
            text: goMarkAsReadFab.text
            delay: Kirigami.Units.toolTipDelay
            visible: goMarkAsReadFab.hovered
        }
    }
}
