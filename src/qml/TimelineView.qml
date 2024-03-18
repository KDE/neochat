// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.qmlmodels
import QtQuick.Window

import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels

import org.kde.neochat
import org.kde.neochat.config
import org.kde.neochat.timeline

QQC2.ScrollView {
    id: root
    required property NeoChatRoom currentRoom
    onCurrentRoomChanged: {
        roomChanging = true;
        roomChangingTimer.restart();
        applicationWindow().hoverLinkIndicator.text = "";
        messageListView.positionViewAtBeginning();
        hasScrolledUpBefore = false;
    }
    property bool roomChanging: false

    /**
     * @brief The TimelineModel to use.
     *
     * Required so that new events can be requested when the end of the current
     * local timeline is reached.
     */
    required property TimelineModel timelineModel

    /**
     * @brief The MessageFilterModel to use.
     *
     * This model has the filtered list of events that should be shown in the timeline.
     */
    required property MessageFilterModel messageFilterModel

    /**
     * @brief The ActionsHandler object to use.
     *
     * This is expected to have the correct room set otherwise messages will be sent
     * to the wrong room.
     */
    required property ActionsHandler actionsHandler

    readonly property bool atYEnd: messageListView.atYEnd

    property alias interactive: messageListView.interactive

    /// Used to determine if scrolling to the bottom should mark the message as unread
    property bool hasScrolledUpBefore: false

    signal focusChatBar

    QQC2.ScrollBar.vertical.interactive: false

    ListView {
        id: messageListView
        // So that delegates can access the actionsHandler properly.
        readonly property ActionsHandler actionsHandler: root.actionsHandler

        readonly property int largestVisibleIndex: count > 0 ? indexAt(contentX + (width / 2), contentY + height - 1) : -1
        readonly property var sectionBannerItem: contentHeight >= height ? itemAtIndex(sectionBannerIndex()) : undefined

        // Spacing needs to be zero or the top sectionLabel overlay will be disrupted.
        // This is because itemAt returns null in the spaces.
        // All spacing should be handled by the delegates themselves
        spacing: 0
        verticalLayoutDirection: ListView.BottomToTop
        clip: true
        interactive: Kirigami.Settings.isMobile
        bottomMargin: Kirigami.Units.largeSpacing + Math.round(Kirigami.Theme.defaultFont.pointSize * 2)

        model: root.messageFilterModel

        onCountChanged: if (root.roomChanging) {
            root.positionViewAtBeginning();
        }

        Timer {
            interval: 1000
            running: messageListView.atYBeginning
            triggeredOnStart: true
            onTriggered: {
                if (messageListView.atYBeginning && root.timelineModel.messageEventModel.canFetchMore(root.timelineModel.index(0, 0))) {
                    root.timelineModel.messageEventModel.fetchMore(root.timelineModel.index(0, 0));
                }
            }
            repeat: true
        }

        // HACK: The view should do this automatically but doesn't.
        onAtYBeginningChanged: if (atYBeginning && root.timelineModel.messageEventModel.canFetchMore(root.timelineModel.index(0, 0))) {
            root.timelineModel.messageEventModel.fetchMore(root.timelineModel.index(0, 0));
        }

        Timer {
            id: roomChangingTimer
            interval: 1000
            onTriggered: {
                root.roomChanging = false;
                markReadIfVisibleTimer.reset();
            }
        }
        onAtYEndChanged: if (!root.roomChanging) {
            if (atYEnd && root.hasScrolledUpBefore) {
                if (QQC2.ApplicationWindow.window && (QQC2.ApplicationWindow.window.visibility !== QQC2.ApplicationWindow.Hidden)) {
                    root.currentRoom.markAllMessagesAsRead();
                }
                root.hasScrolledUpBefore = false;
            } else if (!atYEnd) {
                root.hasScrolledUpBefore = true;
            }
        }

        // Not rendered because the sections are part of the MessageDelegate.qml, this is only so that items have the section property available for use by sectionBanner.
        // This is due to the fact that the ListView verticalLayout is BottomToTop.
        // This also flips the sections which would appear at the bottom but for a timeline they still need to be at the top (bottom from the qml perspective).
        // There is currently no option to put section headings at the bottom in qml.
        section.property: "section"

        function sectionBannerIndex() {
            let center = messageListView.x + messageListView.width / 2;
            let yStart = messageListView.y + messageListView.contentY;
            let index = -1;
            let i = 0;
            while (index === -1 && i < 100) {
                index = messageListView.indexAt(center, yStart + i);
                i++;
            }
            return index;
        }

        footer: Item {
            z: 3
            width: root.width
            visible: messageListView.sectionBannerItem && messageListView.sectionBannerItem.ListView.section !== "" && !Config.blur

            SectionDelegate {
                id: sectionDelegate
                anchors.leftMargin: state === "alignLeft" ? Kirigami.Units.largeSpacing : 0
                state: Config.compactLayout ? "alignLeft" : "alignCenter"
                // Align left when in compact mode and center when using bubbles
                states: [
                    State {
                        name: "alignLeft"
                        AnchorChanges {
                            target: sectionDelegate
                            anchors.horizontalCenter: undefined
                            anchors.left: parent ? parent.left : undefined
                        }
                    },
                    State {
                        name: "alignCenter"
                        AnchorChanges {
                            target: sectionDelegate
                            anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
                            anchors.left: undefined
                        }
                    }
                ]

                width: messageListView.sectionBannerItem ? messageListView.sectionBannerItem.contentItem.width : 0
                labelText: messageListView.sectionBannerItem ? messageListView.sectionBannerItem.ListView.section : ""
                colorSet: Config.compactLayout ? Kirigami.Theme.View : Kirigami.Theme.Window
            }
        }
        footerPositioning: ListView.OverlayHeader

        delegate: EventDelegate {
            room: root.currentRoom
        }

        KirigamiComponents.FloatingButton {
            id: goReadMarkerFab

            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: Kirigami.Units.largeSpacing
            anchors.rightMargin: Kirigami.Units.largeSpacing
            implicitWidth: Kirigami.Units.gridUnit * 2
            implicitHeight: Kirigami.Units.gridUnit * 2

            z: 2
            visible: root.currentRoom && root.currentRoom.hasUnreadMessages && root.currentRoom.readMarkerLoaded
            action: Kirigami.Action {
                onTriggered: {
                    if (!Kirigami.Settings.isMobile) {
                        root.focusChatBar();
                    }
                    messageListView.goToEvent(root.currentRoom.readMarkerEventId);
                }
                icon.name: "go-up"
                shortcut: "Shift+PgUp"
            }

            QQC2.ToolTip {
                text: i18n("Jump to first unread message")
            }
        }
        KirigamiComponents.FloatingButton {
            id: goMarkAsReadFab
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Kirigami.Units.largeSpacing
            anchors.rightMargin: Kirigami.Units.largeSpacing
            implicitWidth: Kirigami.Units.gridUnit * 2
            implicitHeight: Kirigami.Units.gridUnit * 2

            z: 2
            visible: !messageListView.atYEnd
            action: Kirigami.Action {
                onTriggered: {
                    messageListView.goToLastMessage();
                    root.currentRoom.markAllMessagesAsRead();
                }
                icon.name: "go-down"
            }

            QQC2.ToolTip {
                text: i18n("Jump to latest message")
            }
        }

        Component.onCompleted: {
            positionViewAtBeginning();
        }

        DropArea {
            id: dropAreaFile
            anchors.fill: parent
            onDropped: root.currentRoom.mainCache.attachmentPath = drop.urls[0]
            enabled: !Controller.isFlatpak
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

        TypingPane {
            id: typingPane
            visible: root.currentRoom && root.currentRoom.usersTyping.length > 0
            labelText: visible ? i18ncp("Message displayed when some users are typing", "%2 is typing", "%2 are typing", root.currentRoom.usersTyping.length, root.currentRoom.usersTyping.map(user => user.displayName).join(", ")) : ""
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            height: visible ? implicitHeight : 0
            Behavior on height {
                NumberAnimation {
                    property: "height"
                    duration: Kirigami.Units.shortDuration
                    easing.type: Easing.OutCubic
                }
            }
            z: 2
        }

        function goToEvent(eventID) {
            const index = eventToIndex(eventID);
            messageListView.positionViewAtIndex(index, ListView.Center);
            itemAtIndex(index).isTemporaryHighlighted = true;
        }

        HoverActions {
            id: hoverActions
            currentRoom: root.currentRoom
            onFocusChatBar: root.focusChatBar()
        }

        onContentYChanged: {
            if (hoverActions.delegate) {
                hoverActions.delegate.setHoverActionsToDelegate();
            }
        }

        Connections {
            target: root.timelineModel

            function onRowsInserted() {
                markReadIfVisibleTimer.reset();
            }
        }

        Timer {
            id: markReadIfVisibleTimer
            running: messageListView.allUnreadVisible() && applicationWindow().active && (root.currentRoom.timelineSize > 0 || root.currentRoom.allHistoryLoaded)
            interval: 10000
            onTriggered: root.currentRoom.markAllMessagesAsRead()

            function reset() {
                restart();
                running = Qt.binding(function () {
                    return messageListView.allUnreadVisible() && applicationWindow().active && (root.currentRoom.timelineSize > 0 || root.currentRoom.allHistoryLoaded);
                });
            }
        }

        Rectangle {
            FancyEffectsContainer {
                id: fancyEffectsContainer
                anchors.fill: parent
                z: 100

                enabled: Config.showFancyEffects

                function processFancyEffectsReason(fancyEffect) {
                    if (fancyEffect === "snowflake") {
                        fancyEffectsContainer.showSnowEffect();
                    }
                    if (fancyEffect === "fireworks") {
                        fancyEffectsContainer.showFireworksEffect();
                    }
                    if (fancyEffect === "confetti") {
                        fancyEffectsContainer.showConfettiEffect();
                    }
                }

                Connections {
                    //enabled: Config.showFancyEffects
                    target: root.timelineModel.messageEventModel

                    function onFancyEffectsReasonFound(fancyEffect) {
                        fancyEffectsContainer.processFancyEffectsReason(fancyEffect);
                    }
                }

                Connections {
                    enabled: Config.showFancyEffects
                    target: actionsHandler

                    function onShowEffect(fancyEffect) {
                        fancyEffectsContainer.processFancyEffectsReason(fancyEffect);
                    }
                }
            }
        }

        function goToLastMessage() {
            root.currentRoom.markAllMessagesAsRead();
            // scroll to the very end, i.e to messageListView.YEnd
            messageListView.positionViewAtIndex(0, ListView.End);
        }

        function eventToIndex(eventID) {
            const index = root.timelineModel.messageEventModel.eventIdToRow(eventID);
            if (index === -1)
                return -1;
            return root.messageFilterModel.mapFromSource(root.timelineModel.index(index, 0)).row;
        }

        function firstVisibleIndex() {
            let center = messageListView.x + messageListView.width / 2;
            let index = -1;
            let i = 0;
            while (index === -1 && i < 100) {
                index = messageListView.indexAt(center, messageListView.y + messageListView.contentY + i);
                i++;
            }
            return index;
        }

        function lastVisibleIndex() {
            let center = messageListView.x + messageListView.width / 2;
            let index = -1;
            let i = 0;
            while (index === -1 && i < 100) {
                index = messageListView.indexAt(center, messageListView.y + messageListView.contentY + messageListView.height - i);
                i++;
            }
            return index;
        }

        function allUnreadVisible() {
            let readMarkerRow = eventToIndex(root.currentRoom.readMarkerEventId);
            if (readMarkerRow >= 0 && readMarkerRow < firstVisibleIndex() && messageListView.atYEnd) {
                return true;
            }
            return false;
        }

        function setHoverActionsToDelegate(delegate) {
            hoverActions.delegate = delegate;
        }
    }

    function goToLastMessage() {
        messageListView.goToLastMessage();
    }

    function pageUp() {
        const newContentY = messageListView.contentY - messageListView.height / 2;
        const minContentY = messageListView.originY + messageListView.topMargin;
        messageListView.contentY = Math.max(newContentY, minContentY);
        messageListView.returnToBounds();
    }

    function pageDown() {
        const newContentY = messageListView.contentY + messageListView.height / 2;
        const maxContentY = messageListView.originY + messageListView.bottomMargin + messageListView.contentHeight - messageListView.height;
        messageListView.contentY = Math.min(newContentY, maxContentY);
        messageListView.returnToBounds();
    }

    function positionViewAtBeginning() {
        messageListView.positionViewAtBeginning();
    }
}
