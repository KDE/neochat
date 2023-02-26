// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1 as Platform
import Qt.labs.qmlmodels 1.0
import QtQuick.Window 2.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: page

    /// It's not readonly because of the seperate window view.
    property var currentRoom: RoomManager.currentRoom
    property bool loading: page.currentRoom === null || (messageListView.count === 0 && !page.currentRoom.allHistoryLoaded && !page.currentRoom.isInvite)
    /// Used to determine if scrolling to the bottom should mark the message as unread
    property bool hasScrolledUpBefore: false;

    /// Disable cancel shortcut. Used by the seperate window since it provide its own
    /// cancel implementation.
    property bool disableCancelShortcut: false

    title: currentRoom.displayName

    KeyNavigation.left: pageStack.get(0)

    Connections {
        target: RoomManager
        function onCurrentRoomChanged() {
            if(!RoomManager.currentRoom) {
                if(pageStack.lastItem == page) {
                    pageStack.pop()
                }
            } else if (page.currentRoom.isInvite) {
                page.currentRoom.clearInvitationNotification();
            }
        }
    }

    signal switchRoomUp()
    signal switchRoomDown()

    onCurrentRoomChanged: {
        applicationWindow().hoverLinkIndicator.text = "";
        messageListView.positionViewAtBeginning();
        hasScrolledUpBefore = false;
        chatBox.chatBar.forceActiveFocus();
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
            if (loading || !currentRoom.readMarkerLoaded || !applicationWindow().active) {
                restart()
            } else {
                markReadIfVisible()
            }
        }
    }

    ActionsHandler {
        id: actionsHandler
        room: page.currentRoom
    }

    Shortcut {
        sequence: StandardKey.Cancel
        onActivated: applicationWindow().pageStack.get(0).forceActiveFocus()
        enabled: !page.disableCancelShortcut
    }

    Connections {
        target: Controller.activeConnection
        function onJoinedRoom(room, invited) {
            if(page.currentRoom.id === invited.id) {
                RoomManager.enterRoom(room);
            }
        }
    }

    Connections {
        target: currentRoom
        function onShowMessage(messageType, message) {
            page.header.contentItem.text = message;
            page.header.contentItem.type = messageType === ActionsHandler.Error ? Kirigami.MessageType.Error : messageType === ActionsHandler.Positive ? Kirigami.MessageType.Positive : Kirigami.MessageType.Information;
            page.header.visible = true;
        }
    }

    header: QQC2.Control {
        height: visible ? implicitHeight : 0
        visible: false
        padding: Kirigami.Units.smallSpacing
        contentItem: Kirigami.InlineMessage {
            showCloseButton: true
            visible: true
        }
    }

    Kirigami.LoadingPlaceholder {
        id: loadingIndicator
        anchors.centerIn: parent
        visible: loading
    }

    focus: true

    Keys.onTabPressed: {
        if (event.modifiers & Qt.ControlModifier) {
            switchRoomDown();
        }
    }

    Keys.onBacktabPressed: {
        if (event.modifiers & Qt.ControlModifier) {
            switchRoomUp();
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_PageDown && (event.modifiers & Qt.ControlModifier)) {
            event.accepted = true;
            switchRoomDown();
        } else if (event.key === Qt.Key_PageUp && (event.modifiers & Qt.ControlModifier)) {
            event.accepted = true;
            switchRoomUp();
        } else if (!(event.modifiers & Qt.ControlModifier) && event.key < Qt.Key_Escape) {
            event.accepted = true;
            chatBox.chatBar.insertText(event.text);
            chatBox.chatBar.forceActiveFocus();
            return;
        }
    }

    // hover actions on a delegate, activated in TimelineContainer.qml
    Connections {
        target: page.flickable
        enabled: hoverActions.visible
        function onContentYChanged() {
            hoverActions.updateFunction();
        }
    }

    CollapseStateProxyModel {
        id: collapseStateProxyModel
        sourceModel: sortedMessageEventModel
    }

    ListView {
        id: messageListView

        readonly property int largestVisibleIndex: count > 0 ? indexAt(contentX + (width / 2), contentY + height - 1) : -1
        readonly property bool isLoaded: page.width * page.height > 10

        // Spacing needs to be zero or the top sectionLabel overlay will be disrupted.
        // This is because itemAt returns null in the spaces.
        // All spacing should be handled by the delegates themselves
        spacing: 0
        // Ensures that the top item is not covered by sectionBanner if the page is scrolled all the way up
        // topMargin: sectionBanner.height
        verticalLayoutDirection: ListView.BottomToTop
        highlightMoveDuration: 500

        // HACK: Needs to be here because the flickable in ScrollablePage gets all mouse events.
        Kirigami.PlaceholderMessage {
            id: invitation

            visible: currentRoom && currentRoom.isInvite
            anchors.centerIn: parent
            text: i18n("Accept this invitation?")
            RowLayout {
                QQC2.Button {
                    Layout.alignment : Qt.AlignHCenter
                    text: i18n("Reject")

                    onClicked: RoomManager.leaveRoom(page.currentRoom);
                }

                QQC2.Button {
                    Layout.alignment : Qt.AlignHCenter
                    text: i18n("Accept")

                    onClicked: {
                        currentRoom.acceptInvitation();
                    }
                }
            }
        }

        model: !isLoaded ? undefined : collapseStateProxyModel

        MessageEventModel {
            id: messageEventModel

            room: currentRoom
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
                currentRoom.markAllMessagesAsRead();
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

        readonly property var sectionBannerItem: contentHeight >= height ? itemAtIndex(sectionBannerIndex()) : undefined

        function sectionBannerIndex() {
            let center = messageListView.x + messageListView.width / 2;
            let yStart = messageListView.y + messageListView.contentY;
            let index = -1
            let i = 0
            while (index === -1 && i < 100) {
                index = messageListView.indexAt(center, yStart + i);
                i++;
            }
            return index
        }

        footer: SectionDelegate {
            id: sectionBanner

            anchors.left: parent.left
            anchors.leftMargin: messageListView.sectionBannerItem ? messageListView.sectionBannerItem.x : 0
            anchors.right: parent.right

            maxWidth: messageListView.sectionBannerItem ? messageListView.sectionBannerItem.width - Kirigami.Units.largeSpacing * 2 : 0
            z: 3
            visible: messageListView.sectionBannerItem != undefined && messageListView.sectionBannerItem.ListView.section != "" && !Config.blur
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

                        fileDialog.chosen.connect(function(path) {
                            if (!path) {
                                return;
                            }
                            currentRoom.chatBoxAttachmentPath = path;
                        })

                        fileDialog.open()
                    }
                }

                Kirigami.Separator {}

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
                        currentRoom.chatBoxAttachmentPath = localPath;
                        attachDialog.close();
                    }
                }
            }
        }

        Component {
            id: openFileDialog

            OpenFileDialog {
                parentWindow: page.Window.window
            }
        }


        MessageFilterModel {
            id: sortedMessageEventModel

            sourceModel: messageEventModel
        }

        delegate: EventDelegate {}

        QQC2.RoundButton {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: Kirigami.Units.largeSpacing
            anchors.rightMargin: Kirigami.Units.largeSpacing
            implicitWidth: Kirigami.Units.gridUnit * 2
            implicitHeight: Kirigami.Units.gridUnit * 2

            id: goReadMarkerFab

            z: 2
            visible: currentRoom && currentRoom.hasUnreadMessages && currentRoom.readMarkerLoaded
            action: Kirigami.Action {
                onTriggered: {
                    chatBox.chatBar.forceActiveFocus();
                    messageListView.goToEvent(currentRoom.readMarkerEventId)
                }
                icon.name: "go-up"
            }

            QQC2.ToolTip {
                text: i18n("Jump to first unread message")
            }
        }
        QQC2.RoundButton {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Kirigami.Units.largeSpacing
            anchors.rightMargin: Kirigami.Units.largeSpacing
            implicitWidth: Kirigami.Units.gridUnit * 2
            implicitHeight: Kirigami.Units.gridUnit * 2

            id: goMarkAsReadFab

            z: 2
            visible: !messageListView.atYEnd
            action: Kirigami.Action {
                onTriggered: {
                    chatBox.chatBar.forceActiveFocus();
                    goToLastMessage();
                    currentRoom.markAllMessagesAsRead();
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
            onDropped: currentRoom.chatBoxAttachmentPath = drop.urls[0];
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
            id: fullScreenImage

            FullScreenImage {}
        }

        Component {
            id: userDetailDialog

            UserDetailDialog {}
        }

        TypingPane {
            id: typingPane
            visible: !loadingIndicator.visible && currentRoom && currentRoom.usersTyping.length > 0
            labelText: visible ? i18ncp(
                "Message displayed when some users are typing", "%2 is typing", "%2 are typing",
                currentRoom.usersTyping.length,
                currentRoom.usersTyping.map(user => user.displayName).join(", ")
            ) : ""
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

        Item {
            id: hoverActions
            property var event: null
            property bool userMsg: event && event.author.id === Controller.activeConnection.localUserId
            property bool showEdit: event && (userMsg && (event.eventType === MessageEventModel.Emote || event.eventType === MessageEventModel.Message))
            property var delegate: null
            property var bubble: null
            property var hovered: bubble && bubble.hovered
            property var visibleDelayed: (hovered || hoverHandler.hovered) && !Kirigami.Settings.isMobile
            property var updateFunction
            onVisibleDelayedChanged: if (visibleDelayed) {
                visible = true;
            } else {
                // HACK: delay disapearing by 200ms, otherwise this can create some glitches
                // See https://invent.kde.org/network/neochat/-/issues/333
                hoverActionsTimer.restart();
            }
            Timer {
                id: hoverActionsTimer
                interval: 200
                onTriggered: hoverActions.visible = hoverActions.visibleDelayed;
            }

            property int childOffset: userMsg && Config.showLocalMessagesOnRight && !Config.compactLayout ? (bubble ? bubble.width : 0) - childWidth : Math.max((bubble ? bubble.width : 0) - childWidth, 0)
            x: delegate && bubble ? (delegate.x + bubble.x + Kirigami.Units.largeSpacing + childOffset - (Config.compactLayout ? Kirigami.Units.gridUnit * 3: 0) - (userMsg && !Config.compactLayout ? Kirigami.Units.gridUnit : 0)) : 0
            y: bubble ? bubble.mapToItem(parent, 0, 0).y - hoverActions.childHeight + Kirigami.Units.smallSpacing: 0;

            visible: false

            property alias childWidth: hoverActionsRow.width
            property alias childHeight: hoverActionsRow.height

            RowLayout {
                id: hoverActionsRow
                z: 4
                spacing: 0
                HoverHandler {
                    id: hoverHandler
                    margin: Kirigami.Units.smallSpacing
                }
                Kirigami.Icon {
                    source: "security-high"
                    width: height
                    height: parent.height
                    visible: hoverActions.event ? hoverActions.event.verified : false
                    HoverHandler {
                        id: hover
                    }
                    QQC2.ToolTip.text: i18n("This message was sent from a verified device")
                    QQC2.ToolTip.visible: hover.hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                QQC2.Button {
                    QQC2.ToolTip.text: i18n("React")
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    icon.name: "preferences-desktop-emoticons"

                    onClicked: emojiDialog.open();
                    EmojiDialog {
                        id: emojiDialog
                        showQuickReaction: true
                        onChosen: {
                            page.currentRoom.toggleReaction(hoverActions.event.eventId, emoji);
                            chatBox.chatBar.forceActiveFocus();
                        }
                    }
                }
                QQC2.Button {
                    QQC2.ToolTip.text: i18n("Edit")
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    visible: hoverActions.showEdit
                    icon.name: "document-edit"
                    onClicked: {
                        currentRoom.chatBoxEditId = hoverActions.event.eventId;
                        currentRoom.chatBoxReplyId = "";
                    }
                }
                QQC2.Button {
                    QQC2.ToolTip.text: i18n("Reply")
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    icon.name: "mail-replied-symbolic"
                    onClicked: {
                        currentRoom.chatBoxReplyId = hoverActions.event.eventId;
                        currentRoom.chatBoxEditId = "";
                        chatBox.chatBar.forceActiveFocus();
                    }
                }
            }
        }
    }


    footer: ChatBox {
        id: chatBox
        visible: !invitation.visible && !(messageListView.count === 0 && !currentRoom.allHistoryLoaded)
        width: parent.width
        onMessageSent: {
            if (!messageListView.atYEnd) {
                goToLastMessage();
            }
        }
    }

    background: FancyEffectsContainer {
        id: fancyEffectsContainer
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
            enabled: Config.showFancyEffects
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


    function warning(title, message) {
        page.header.contentItem.text = `${title}<br />${message}`;
        page.header.contentItem.type =  Kirigami.MessageType.Warning;
        page.header.visible = true;
    }

    function showUserDetail(user) {
        userDetailDialog.createObject(QQC2.ApplicationWindow.overlay, {
            room: currentRoom,
            user: user,
        }).open();
    }

    function goToLastMessage() {
        currentRoom.markAllMessagesAsRead()
        // scroll to the very end, i.e to messageListView.YEnd
        messageListView.positionViewAtIndex(0, ListView.End)
    }

    function eventToIndex(eventID) {
        const index = messageEventModel.eventIDToIndex(eventID)
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
        let readMarkerRow = eventToIndex(currentRoom.readMarkerEventId)
        if (readMarkerRow >= 0 && readMarkerRow < firstVisibleIndex() && messageListView.atYEnd) {
            currentRoom.markAllMessagesAsRead()
        }
    }

    /// Open message context dialog for file and videos
    function openFileContext(event, file) {
        const contextMenu = fileDelegateContextMenu.createObject(page, {
            author: event.author,
            message: event.message,
            eventId: event.eventId,
            source: event.source,
            file: file,
            mimeType: event.mimeType,
            progressInfo: event.progressInfo,
            plainMessage: event.message,
        });
        contextMenu.open();
    }

    /// Open context menu for normal message
    function openMessageContext(event, selectedText, plainMessage) {
        const contextMenu = messageDelegateContextMenu.createObject(page, {
            selectedText: selectedText,
            author: event.author,
            message: event.display,
            eventId: event.eventId,
            formattedBody: event.formattedBody,
            source: event.source,
            eventType: event.eventType,
            plainMessage: plainMessage,
        });
        contextMenu.open();
    }
}
