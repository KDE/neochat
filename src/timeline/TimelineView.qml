// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat
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
     * @brief The TimelineMarkReadCondition to use for when messages should be marked as read automatically.
     */
    required property int markReadCondition

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

    /**
     * @brief Tell the view to resettle again as needed.
     */
    function resetViewSettling() {
        _private.viewHasSettled = false;
    }

    ListView {
        id: messageListView

        // HACK: Use this instead of atYEnd to handle cases like -643.2 at height of 643 not being counted as "at the beginning"
        readonly property bool closeToYEnd: -Math.round(contentY) >= height

        onHeightChanged: {
            // HACK: Fix a bug where Qt doesn't resettle the view properly when the pinned messages changes our height
            // We basically want to resettle back at the start if:
            // * The user hasn't scrolled before (obviously) *and* that scroll is actually somewhere other than the beginning
            // * This is the first height change
            if (!_private.viewHasSettled && (!_private.hasScrolledUpBefore || closeToYEnd)) {
                positionViewAtBeginning();
                _private.viewHasSettled = true;
            }
        }

        /**
         * @brief Whether all unread messages in the timeline are visible.
         */
        function allUnreadVisible() {
            let readMarkerRow = model.readMarkerIndex?.row ?? -1;
            if (readMarkerRow >= 0 && readMarkerRow < oldestVisibleIndex() && closeToYEnd) {
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
            sequences: [ StandardKey.Cancel ]
            onActivated: {
                if (!messageListView.closeToYEnd || !_private.room.partiallyReadStats.empty()) {
                    messageListView.positionViewAtBeginning();
                } else {
                    (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).get(0).forceActiveFocus();
                }
            }
        }

        Connections {
            target: messageListView.model.sourceModel.timelineMessageModel

            function onModelAboutToBeReset() {
                (root.QQC2.ApplicationWindow.window as Main).hoverLinkIndicator.text = "";
                _private.hasScrolledUpBefore = false;
                _private.viewHasSettled = false;
            }

            function onReadMarkerAdded() {
                if (root.markReadCondition == LibNeoChat.TimelineMarkReadCondition.EntryVisible && messageListView.allUnreadVisible()) {
                    _private.room.markAllMessagesAsRead();
                }
            }

            function onNewLocalUserEventAdded() {
                messageListView.positionViewAtBeginning();
                _private.room.markAllMessagesAsRead();
            }

            function onRoomAboutToChange(oldRoom, newRoom) {
                if (root.markReadCondition == LibNeoChat.TimelineMarkReadCondition.Exit ||
                    (root.markReadCondition == LibNeoChat.TimelineMarkReadCondition.ExitVisible && messageListView.allUnreadVisible())
                ) {
                    oldRoom.markAllMessagesAsRead();
                }
            }

            function onRoomChanged(oldRoom, newRoom) {
                if (root.markReadCondition == LibNeoChat.TimelineMarkReadCondition.Entry) {
                    newRoom.markAllMessagesAsRead();
                }
            }
        }

        onCloseToYEndChanged: {
            // Don't care about this until the view has settled first.
            if (!_private.viewHasSettled) {
                return;
            }

            if (closeToYEnd && _private.hasScrolledUpBefore) {
                if (QQC2.ApplicationWindow.window && (QQC2.ApplicationWindow.window.visibility !== QQC2.ApplicationWindow.Hidden)) {
                    _private.room.markAllMessagesAsRead();
                }
                _private.hasScrolledUpBefore = false;
            } else if (!closeToYEnd) {
                _private.hasScrolledUpBefore = true;
            }
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
            visible: !_private.room?.partiallyReadStats.empty()

            text: _private.room.readMarkerLoaded ? i18nc("@action:button", "Jump to first unread message") : i18nc("@action:button", "Jump to oldest loaded message")
            icon.name: "go-up"
            onClicked: {
                goReadMarkerFab.textChanged()
                root.goToEvent(_private.room.lastFullyReadEventId);
            }
            Kirigami.Action {
                shortcut: "Shift+PgUp"
                onTriggered: goReadMarkerFab.clicked()
            }

            QQC2.ToolTip.text: goReadMarkerFab.text
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.visible: goReadMarkerFab.hovered
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
            visible: !messageListView.closeToYEnd

            text: i18nc("@action:button", "Jump to latest message")

            onClicked: {
                messageListView.positionViewAtBeginning();
                _private.room.markAllMessagesAsRead();
            }

            icon.name: "go-down"
            Kirigami.Action {
                shortcut: "Shift+PgDown"
                onTriggered: goMarkAsReadFab.clicked()
            }

            QQC2.ToolTip.text: goMarkAsReadFab.text
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.visible: goMarkAsReadFab.hovered
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
                text: i18nc("@info:placeholder", "Drag items here to share them")
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

        // Used to determine if the view has settled and should stop moving
        property bool viewHasSettled: false
    }
}
