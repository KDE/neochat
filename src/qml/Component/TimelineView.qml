// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1 as Platform
import Qt.labs.qmlmodels 1.0
import QtQuick.Window 2.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

QQC2.ScrollView {
    id: root
    required property NeoChatRoom currentRoom
    readonly property bool isLoaded: root.width * root.height > 10
    readonly property bool atYEnd: messageListView.atYEnd

    signal focusChatBox()

    ListView {
        id: messageListView
        readonly property int largestVisibleIndex: count > 0 ? indexAt(contentX + (width / 2), contentY + height - 1) : -1
        readonly property var sectionBannerItem: contentHeight >= height ? itemAtIndex(sectionBannerIndex()) : undefined

        // Spacing needs to be zero or the top sectionLabel overlay will be disrupted.
        // This is because itemAt returns null in the spaces.
        // All spacing should be handled by the delegates themselves
        spacing: 0
        // Ensures that the top item is not covered by sectionBanner if the page is scrolled all the way up
        // topMargin: sectionBanner.height
        verticalLayoutDirection: ListView.BottomToTop
        highlightMoveDuration: 500
        clip: true
        interactive: Kirigami.Settings.isMobile
        bottomMargin: Kirigami.Units.largeSpacing + Math.round(Kirigami.Theme.defaultFont.pointSize * 2)

        model: !isLoaded ? undefined : collapseStateProxyModel

        MessageEventModel {
            id: messageEventModel
            room: root.currentRoom
        }

        CollapseStateProxyModel {
            id: collapseStateProxyModel
            sourceModel: MessageFilterModel {
                id: sortedMessageEventModel
                sourceModel: messageEventModel
            }
        }

        Timer {
            interval: 1000
            running: messageListView.atYBeginning
            triggeredOnStart: true
            onTriggered: {
                if (messageListView.atYBeginning && messageEventModel.canFetchMore(messageEventModel.index(0, 0))) {
                    messageEventModel.fetchMore(messageEventModel.index(0, 0));
                }
            }
            repeat: true
        }

        // HACK: The view should do this automatically but doesn't.
        onAtYBeginningChanged: if (atYBeginning && messageEventModel.canFetchMore(messageEventModel.index(0, 0))) {
            messageEventModel.fetchMore(messageEventModel.index(0, 0));
        }

        onAtYEndChanged: if (atYEnd && hasScrolledUpBefore) {
            if (QQC2.ApplicationWindow.window && (QQC2.ApplicationWindow.window.visibility !== QQC2.ApplicationWindow.Hidden)) {
                root.currentRoom.markAllMessagesAsRead();
            }
            hasScrolledUpBefore = false;
        } else if (!atYEnd) {
            hasScrolledUpBefore = true;
        }

        // Not rendered because the sections are part of the TimelineContainer.qml, this is only so that items have the section property available for use by sectionBanner.
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

        footer: SectionDelegate {
            id: sectionBanner

            anchors.left: parent.left
            anchors.leftMargin: messageListView.sectionBannerItem ? messageListView.sectionBannerItem.x : 0
            anchors.right: parent.right

            maxWidth: Config.compactLayout ? messageListView.width : (messageListView.sectionBannerItem ? messageListView.sectionBannerItem.width - Kirigami.Units.largeSpacing * 2 : 0)
            z: 3
            visible: !!messageListView.sectionBannerItem && messageListView.sectionBannerItem.ListView.section !== "" && !Config.blur
            labelText: messageListView.sectionBannerItem ? messageListView.sectionBannerItem.ListView.section : ""
        }
        footerPositioning: ListView.OverlayHeader

        QQC2.Popup {
            anchors.centerIn: parent

            id: attachDialog

            padding: 16

            contentItem: RowLayout {
                QQC2.ToolButton {
                    Layout.preferredWidth: 160
                    Layout.fillHeight: true

                    icon.name: 'mail-attachment'

                    text: i18n("Choose local file")

                    onClicked: {
                        attachDialog.close()

                        var fileDialog = openFileDialog.createObject(QQC2.ApplicationWindow.overlay)

                        fileDialog.chosen.connect(function (path) {
                            if (!path) {
                                return;
                            }
                            root.currentRoom.chatBoxAttachmentPath = path;
                        })

                        fileDialog.open()
                    }
                }

                Kirigami.Separator {
                }

                QQC2.ToolButton {
                    Layout.preferredWidth: 160
                    Layout.fillHeight: true

                    padding: 16

                    icon.name: 'insert-image'
                    text: i18n("Clipboard image")
                    onClicked: {
                        const localPath = Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/screenshots/" + (new Date()).getTime() + ".png"
                        if (!Clipboard.saveImage(localPath)) {
                            return;
                        }
                        root.currentRoom.chatBoxAttachmentPath = localPath;
                        attachDialog.close();
                    }
                }
            }
        }

        Component {
            id: openFileDialog

            OpenFileDialog {
                parentWindow: Window.window
            }
        }

        delegate: EventDelegate {
            room: root.currentRoom
        }

        QQC2.RoundButton {
            id: goReadMarkerFab

            Accessible.name: i18n("Jump to first unread message")

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
                        root.focusChatBox();
                    }
                    messageListView.goToEvent(root.currentRoom.readMarkerEventId)
                }
                icon.name: "go-up"
                shortcut: "Shift+PgUp"
            }

            QQC2.ToolTip {
                Accessible.ignored: true
                text: i18n("Jump to first unread message")
            }
        }
        QQC2.RoundButton {
            Accessible.name: i18n("Jump to latest message")
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
                Accessible.ignored: true
                text: i18n("Jump to latest message")
            }
        }

        Component.onCompleted: {
            positionViewAtBeginning();
        }

        DropArea {
            id: dropAreaFile
            anchors.fill: parent
            onDropped: root.currentRoom.chatBoxAttachmentPath = drop.urls[0]
            ;
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

        Component {
            id: messageDelegateContextMenu

            MessageDelegateContextMenu {}
        }

        Component {
            id: fileDelegateContextMenu

            FileDelegateContextMenu {}
        }

        Component {
            id: userDetailDialog

            UserDetailDialog {}
        }

        TypingPane {
            id: typingPane
            visible: root.currentRoom && root.currentRoom.usersTyping.length > 0
            labelText: visible ? i18ncp(
                "Message displayed when some users are typing", "%2 is typing", "%2 are typing",
                root.currentRoom.usersTyping.length,
                root.currentRoom.usersTyping.map(user => user.displayName).join(", ")
            ) :
            ""
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
            const index = eventToIndex(eventID)
            messageListView.positionViewAtIndex(index, ListView.Center)
            itemAtIndex(index).isTemporaryHighlighted = true
        }

        HoverActions {
            id: hoverActions

            property var delegate: null

            x: delegate ? delegate.x + delegate.bubbleX : 0
            y: delegate ? delegate.mapToItem(parent, 0, 0).y + delegate.bubbleY - height + Kirigami.Units.smallSpacing : 0
            width: delegate.bubbleWidth

            showActions: delegate && delegate.hovered
            verified: delegate && delegate.verified
            editable: delegate && delegate.author.isLocalUser && (delegate.delegateType === MessageEventModel.Emote || delegate.delegateType === MessageEventModel.Message)

            onReactClicked: (emoji) => {
                root.currentRoom.toggleReaction(delegate.eventId, emoji);
                if (!Kirigami.Settings.isMobile) {
                    root.focusChatBox();
                }
            }
            onEditClicked: {
                root.currentRoom.chatBoxEditId = delegate.eventId;
                root.currentRoom.chatBoxReplyId = "";
            }
            onReplyClicked: {
                root.currentRoom.chatBoxReplyId = delegate.eventId;
                root.currentRoom.chatBoxEditId = "";
                root.focusChatBox();
            }
        }

        onContentYChanged: {
            if (hoverActions.delegate) {
                hoverActions.delegate.setHoverActionsToDelegate();
            }
        }

        Connections {
            target: messageEventModel

            function onRowsInserted() {
                markReadIfVisibleTimer.restart()
            }
        }

        Timer {
            id: markReadIfVisibleTimer
            interval: 1000
            onTriggered: {
                if (loading || !root.currentRoom.readMarkerLoaded || !applicationWindow().active) {
                    restart()
                } else {
                    messageListView.markReadIfVisible()
                }
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
                        fancyEffectsContainer.showSnowEffect()
                    }
                    if (fancyEffect === "fireworks") {
                        fancyEffectsContainer.showFireworksEffect()
                    }
                    if (fancyEffect === "confetti") {
                        fancyEffectsContainer.showConfettiEffect()
                    }
                }

                Connections {
                    //enabled: Config.showFancyEffects
                    target: messageEventModel

                    function onFancyEffectsReasonFound(fancyEffect) {
                        fancyEffectsContainer.processFancyEffectsReason(fancyEffect)
                    }
                }

                Connections {
                    enabled: Config.showFancyEffects
                    target: actionsHandler

                    function onShowEffect(fancyEffect) {
                        fancyEffectsContainer.processFancyEffectsReason(fancyEffect)
                    }
                }
            }
        }

        MediaMessageFilterModel {
            id: mediaMessageFilterModel
            sourceModel: collapseStateProxyModel
        }

        Component {
            id: maximizeComponent
            NeochatMaximizeComponent {
                model: mediaMessageFilterModel
            }
        }

        function showMaximizedMedia(index) {
            var popup = maximizeComponent.createObject(QQC2.ApplicationWindow.overlay, {
                    initialIndex: index === -1 ? -1 : mediaMessageFilterModel.getRowForSourceItem(index)
                })
                popup.closed.connect(() => {
                    messageListView.interactive = true
                    popup.destroy()
                })
                popup.open()
        }

        function showUserDetail(user) {
            userDetailDialog.createObject(QQC2.ApplicationWindow.overlay, {
                room: root.currentRoom,
                user: root.currentRoom.getUser(user.id),
            }).open();
        }

        function goToLastMessage() {
            root.currentRoom.markAllMessagesAsRead()
            // scroll to the very end, i.e to messageListView.YEnd
            messageListView.positionViewAtIndex(0, ListView.End)
        }

        function eventToIndex(eventID) {
            const index = messageEventModel.eventIdToRow(eventID)
            if (index === -1)
                return -1
            return sortedMessageEventModel.mapFromSource(messageEventModel.index(index, 0)).row
        }

        function firstVisibleIndex() {
            let center = messageListView.x + messageListView.width / 2;
            let index = -1
            let i = 0
            while (index === -1 && i < 100) {
                index = messageListView.indexAt(center, messageListView.y + messageListView.contentY + i);
                i++;
            }
            return index
        }

        function lastVisibleIndex() {
            let center = messageListView.x + messageListView.width / 2;
            let index = -1
            let i = 0
            while (index === -1 && i < 100) {
                index = messageListView.indexAt(center, messageListView.y + messageListView.contentY + messageListView.height - i);
                i++
            }
            return index;
        }

        // Mark all messages as read if all unread messages are visible to the user
        function markReadIfVisible() {
            let readMarkerRow = eventToIndex(root.currentRoom.readMarkerEventId)
            if (readMarkerRow >= 0 && readMarkerRow < firstVisibleIndex() && messageListView.atYEnd) {
                root.currentRoom.markAllMessagesAsRead()
            }
        }

        function setHoverActionsToDelegate(delegate) {
            hoverActions.delegate = delegate
        }
    }

    function goToLastMessage() {
        messageListView.goToLastMessage()
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
        messageListView.positionViewAtBeginning()
    }
}
