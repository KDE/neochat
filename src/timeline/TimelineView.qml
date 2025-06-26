// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat.libneochat as LibNeoChat

QQC2.ScrollView {
    id: root

    /**
     * @brief The MessageFilterModel to use.
     *
     * This model has the filtered list of events that should be shown in the timeline.
     */
    required property MessageFilterModel messageFilterModel

    /**
     * @brief Whether the timeline ListView is interactive.
     */
    property alias interactive: messageListView.interactive

    /**
     * @brief Whether the compact message layout is to be used.
     */
    required property bool compactLayout

    /**
     * @brief Whether the compact message layout is to be used.
     */
    property bool fileDropEnabled: true

    /**
     * @brief Shift the view to the given event ID.
     */
    function goToEvent(eventId) {
        const index = messageListView.model.indexforEventId(eventId)
        if (!index.valid) {
            messageListView.positionViewAtEnd();
            return;
        }
        messageListView.positionViewAtIndex(index.row, ListView.Center);
        messageListView.itemAtIndex(index.row).isTemporaryHighlighted = true;
    }

    /**
     * @brief Shift the view to the latest message.
     *
     * All messages will be marked as read.
     */
    function goToLastMessage() {
        _private.room.markAllMessagesAsRead();
        messageListView.positionViewAtBeginning();
    }

    /**
     * @brief Move the timeline up a page.
     */
    function pageUp() {
        const newContentY = messageListView.contentY - messageListView.height / 2;
        const minContentY = messageListView.originY + messageListView.topMargin;
        messageListView.contentY = Math.max(newContentY, minContentY);
        messageListView.returnToBounds();
    }

    /**
     * @brief Move the timeline down a page.
     */
    function pageDown() {
        const newContentY = messageListView.contentY + messageListView.height / 2;
        const maxContentY = messageListView.originY + messageListView.bottomMargin + messageListView.contentHeight - messageListView.height;
        messageListView.contentY = Math.min(newContentY, maxContentY);
        messageListView.returnToBounds();
    }

    QQC2.ScrollBar.vertical.interactive: false

    ListView {
        id: messageListView

        /**
         * @brief Whether all unread messages in the timeline are visible.
         */
        function allUnreadVisible() {
            let readMarkerRow = model.readMarkerIndex?.row ?? -1;
            if (readMarkerRow >= 0 && readMarkerRow < oldestVisibleIndex() && atYEnd) {
                return true;
            }
            return false;
        }

        /**
         * @brief Get the oldest visible message.
         */
        function oldestVisibleIndex() {
            let center = x + width / 2;
            let index = -1;
            let i = 0;
            while (index === -1 && i < 100) {
                index = indexAt(center, y + contentY + i);
                i++;
            }
            return index;
        }

        /**
         * @brief Get the newest visible message.
         */
        function newestVisibleIndex() {
            let center = x + width / 2;
            let index = -1;
            let i = 0;
            while (index === -1 && i < 100) {
                index = indexAt(center, y + contentY + height - i);
                i++;
            }
            return index;
        }

        spacing: 0
        verticalLayoutDirection: ListView.BottomToTop
        clip: true
        interactive: Kirigami.Settings.isMobile

        Shortcut {
            sequence: StandardKey.Cancel
            onActivated: {
                if (!messageListView.atYEnd || !_private.room.partiallyReadStats.empty()) {
                    messageListView.positionViewAtBeginning();
                } else {
                    applicationWindow().pageStack.get(0).forceActiveFocus();
                }
            }
        }

        Component.onCompleted: {
            positionViewAtBeginning();
        }
        Connections {
            target: messageListView.model.sourceModel.timelineMessageModel

            function onModelAboutToBeReset() {
                applicationWindow().hoverLinkIndicator.text = "";
                _private.hasScrolledUpBefore = false;
            }

            function onModelResetComplete() {
                messageListView.positionViewAtBeginning();
            }

            function onReadMarkerAdded() {
                if (messageListView.allUnreadVisible()) {
                    _private.room.markAllMessagesAsRead();
                }
            }

            function onNewLocalUserEventAdded() {
                messageListView.positionViewAtBeginning();
                _private.room.markAllMessagesAsRead();
            }
        }

        onAtYEndChanged: if (atYEnd && _private.hasScrolledUpBefore) {
            if (QQC2.ApplicationWindow.window && (QQC2.ApplicationWindow.window.visibility !== QQC2.ApplicationWindow.Hidden)) {
                _private.room.markAllMessagesAsRead();
            }
            _private.hasScrolledUpBefore = false;
        } else if (!atYEnd) {
            _private.hasScrolledUpBefore = true;
        }

        model: root.messageFilterModel
        delegate: EventDelegate {
            room: _private.room
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
            visible: (!_private.room?.partiallyReadStats.empty())

            text: _private.room.readMarkerLoaded ? i18n("Jump to first unread message") : i18n("Jump to oldest loaded message")
            action: Kirigami.Action {
                onTriggered: {
                    goReadMarkerFab.textChanged()
                    root.goToEvent(_private.room.lastFullyReadEventId);
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
                    messageListView.positionViewAtBeginning();
                    _private.room.markAllMessagesAsRead();
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

        DropArea {
            id: dropAreaFile
            anchors.fill: parent
            onDropped: drop => { _private.room.mainCache.attachmentPath = drop.urls[0] }
            enabled: root.fileDropEnabled
        }

        QQC2.Pane {
            visible: dropAreaFile.containsDrag
            anchors {
                fill: parent
                margins: Kirigami.Units.gridUnit
            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                text: i18n("Drag items here to share them")
            }
        }

        RowLayout {
            id: typingPaneContainer
            visible: _private.room && _private.room.otherMembersTyping.length > 0
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: visible ? typingPane.implicitHeight : 0
            z: 2
            Behavior on height {
                NumberAnimation {
                    property: "height"
                    duration: Kirigami.Units.shortDuration
                    easing.type: Easing.OutCubic
                }
            }
            RowLayout {
                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                Layout.maximumWidth: typingPaneSizeHelper.availableWidth
                TypingPane {
                    id: typingPane
                    labelText: visible ? i18ncp("Message displayed when some users are typing", "%2 is typing", "%2 are typing", _private.room.otherMembersTyping.length, _private.room.otherMembersTyping.map(member => member.displayName).join(", ")) : ""
                }
            }

            LibNeoChat.DelegateSizeHelper {
                id: typingPaneSizeHelper
                parentItem: typingPaneContainer
                startBreakpoint: Kirigami.Units.gridUnit * 46
                endBreakpoint: Kirigami.Units.gridUnit * 66
                startPercentWidth: 100
                endPercentWidth: root.compactLayout ? 100 : 85
                maxWidth: root.compactLayout ? -1 : Kirigami.Units.gridUnit * 60
            }
        }

        Timer {
            interval: 1000
            running: messageListView.atYBeginning
            triggeredOnStart: true
            onTriggered: {
                if (messageListView.atYBeginning && messageListView.model.sourceModel.canFetchMore(messageListView.model.index(0, 0))) {
                    messageListView.model.sourceModel.fetchMore(messageListView.model.index(0, 0));
                }
            }
            repeat: true
        }
    }

    QtObject {
        id: _private

        // Get the room from the model so we always have the one its using (this
        // may not be the case just after RoomManager.currentRoom changes while
        // the model does the switch over).
        readonly property LibNeoChat.NeoChatRoom room: messageListView.model.sourceModel.timelineMessageModel.room

        // Used to determine if scrolling to the bottom should mark the message as unread
        property bool hasScrolledUpBefore: false
    }
}
