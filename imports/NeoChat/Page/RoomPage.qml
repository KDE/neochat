// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1 as Platform
import Qt.labs.qmlmodels 1.0

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0
import NeoChat.Component 1.0
import NeoChat.Component.ChatBox 1.0
import NeoChat.Component.Timeline 1.0
import NeoChat.Dialog 1.0
import NeoChat.Menu.Timeline 1.0

Kirigami.ScrollablePage {
    id: page

    /// It's not readonly because of the seperate window view.
    property var currentRoom: RoomManager.currentRoom

    title: currentRoom.displayName

    signal switchRoomUp()
    signal switchRoomDown()

    onCurrentRoomChanged: ChatBoxHelper.clearEditReply()

    ActionsHandler {
        id: actionsHandler
        room: page.currentRoom
        connection: Controller.activeConnection
    }

    Connections {
        target: Controller.activeConnection
        function onJoinedRoom(room) {
            if(room.id === invitation.id) {
                RoomManager.enterRoom(room);
            }
        }
    }

    Connections {
        target: actionsHandler
        onShowMessage: {
            page.header.contentItem.text = message;
            page.header.contentItem.type = messageType === ActionsHandler.Error ? Kirigami.MessageType.Error : Kirigami.MessageType.Information;
            page.header.contentItem.visible = true;
        }
    }

    header: QQC2.Control {
        height: visible ? implicitHeight : 0
        visible: contentItem.visible
        padding: Kirigami.Units.smallSpacing
        contentItem: Kirigami.InlineMessage {
            showCloseButton: true
            visible: false
        }
    }

    Kirigami.PlaceholderMessage {
        id: invitation

        property var id

        visible: currentRoom && currentRoom.isInvite
        anchors.centerIn: parent
        text: i18n("Accept this invitation?")
        RowLayout {
            QQC2.Button {
                Layout.alignment : Qt.AlignHCenter
                text: i18n("Reject")

                onClicked: RoomManager.leave(page.currentRoom);
            }

            QQC2.Button {
                Layout.alignment : Qt.AlignHCenter
                text: i18n("Accept")

                onClicked: {
                    currentRoom.acceptInvitation();
                    invitation.id = currentRoom.id
                    currentRoom = null
                }
            }
        }
    }

    Kirigami.PlaceholderMessage {
        id: loadingIndicator
        anchors.centerIn: parent
        visible: page.currentRoom === null || (messageListView.count === 0 && !page.currentRoom.allHistoryLoaded && !page.currentRoom.isInvite)
        text: i18n("Loading")
        QQC2.BusyIndicator {
            running: loadingIndicator.visible
            Layout.alignment: Qt.AlignHCenter
        }
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
            switchRoomUp();
        } else if (event.key === Qt.Key_PageUp && (event.modifiers & Qt.ControlModifier)) {
            switchRoomDown();
        } else if (!(event.modifiers & Qt.ControlModifier) && event.key < Qt.Key_Escape) {
            event.accepted = true;
            chatBox.addText(event.text);
            chatBox.focusInputField();
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

    Item {
        id: hoverActions
        property var event
        property bool showEdit: event && (event.author.id === Controller.activeConnection.localUserId && (event.eventType === "emote" || event.eventType === "message"))
        property var bubble
        property var hovered: bubble && bubble.hovered
        property var visibleDelayed: (hovered || hoverHandler.hovered) && !Kirigami.Settings.isMobile
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
        x: bubble ? (bubble.x + Kirigami.Units.largeSpacing + Math.max(bubble.width - childWidth, 0)) : 0
        y: bubble ? bubble.mapToItem(page, 0, -Kirigami.Units.largeSpacing - hoverActions.childHeight * 1.5).y : 0
        visible: false

        property var updateFunction

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

            QQC2.Button {
                QQC2.ToolTip.text: i18n("React")
                QQC2.ToolTip.visible: hovered
                icon.name: "preferences-desktop-emoticons"
                onClicked: emojiDialog.open();
                EmojiDialog {
                    id: emojiDialog
                    onReact: {
                        page.currentRoom.toggleReaction(hoverActions.event.eventId, emoji);
                        chatBox.focusInputField();
                    }
                }
            }
            QQC2.Button {
                QQC2.ToolTip.text: i18n("Edit")
                QQC2.ToolTip.visible: hovered
                visible: hoverActions.showEdit
                icon.name: "document-edit"
                onClicked: {
                    if (hoverActions.showEdit) {
                        ChatBoxHelper.edit(hoverActions.event.message, hoverActions.event.formattedBody, hoverActions.event.eventId)
                    }
                    chatBox.focusInputField();
                }
            }
            QQC2.Button {
                QQC2.ToolTip.text: i18n("Reply")
                QQC2.ToolTip.visible: hovered
                icon.name: "mail-replied-symbolic"
                onClicked: {
                    ChatBoxHelper.replyToMessage(hoverActions.event.eventId, hoverActions.event.message, hoverActions.event.author);
                    chatBox.focusInputField();
                }
            }
        }
    }

    ListView {
        id: messageListView
        pixelAligned: true
        visible: !invitation.visible

        readonly property int largestVisibleIndex: count > 0 ? indexAt(contentX + (width / 2), contentY + height - 1) : -1
        readonly property bool isLoaded: page.width * page.height > 10

        spacing: Kirigami.Units.smallSpacing
        reuseItems: true

        verticalLayoutDirection: ListView.BottomToTop
        highlightMoveDuration: 500

        model: !isLoaded ? undefined : sortedMessageEventModel

        MessageEventModel {
            id: messageEventModel

            room: currentRoom
        }

        // HACK: The view should do this automatically but doesn't.
        onAtYBeginningChanged: if (atYBeginning && messageEventModel.canFetchMore(messageEventModel.index(0, 0))) {
            messageEventModel.fetchMore(messageEventModel.index(0, 0));
        }

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
                            if (!path) return

                            ChatBoxHelper.attachmentPath = path;
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
                        ChatBoxHelper.attachmentPath = localPath;
                        attachDialog.close();
                    }
                }
            }
        }

        Component {
            id: openFileDialog

            OpenFileDialog {}
        }


        MessageFilterModel {
            id: sortedMessageEventModel

            sourceModel: messageEventModel
        }

        delegate: DelegateChooser {
            id: timelineDelegateChooser
            role: "eventType"

            property bool delegateLoaded: true
            ListView.onPooled: delegateLoaded = false
            ListView.onReused: delegateLoaded = true

            DelegateChoice {
                roleValue: "state"
                delegate: QQC2.Control {
                    leftPadding: Kirigami.Units.gridUnit * 1.5 + Kirigami.Units.smallSpacing
                    topPadding: 0
                    bottomPadding: 0
                    contentItem: StateDelegate { }
                    implicitWidth: messageListView.width - Kirigami.Units.largeSpacing
                }
            }

            DelegateChoice {
                roleValue: "emote"
                delegate: TimelineContainer {
                    id: emoteContainer
                    width: messageListView.width
                    isLoaded: timelineDelegateChooser.delegateLoaded
                    isEmote: true
                    onReplyClicked: goToEvent(eventID)
                    hoverComponent: hoverActions

                    innerObject: TextDelegate {
                        isEmote: true
                        Layout.fillWidth: !Config.showAvatarInTimeline
                        Layout.maximumWidth: emoteContainer.bubbleMaxWidth
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        Layout.leftMargin: Config.showAvatarInTimeline ? Kirigami.Units.largeSpacing : 0
                        Layout.bottomMargin: Kirigami.Units.largeSpacing * 2
                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: openMessageContext(author, model.message, eventId, toolTip, eventType, model.formattedBody)
                        }
                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onLongPressed: openMessageContext(author, model.message, eventId, toolTip, eventType, model.formattedBody)
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "message"
                delegate: TimelineContainer {
                    id: messageContainer
                    width: messageListView.width
                    isLoaded: timelineDelegateChooser.delegateLoaded
                    onReplyClicked: goToEvent(eventID)
                    hoverComponent: hoverActions

                    innerObject: TextDelegate {
                        Layout.fillWidth: !Config.showAvatarInTimeline
                        Layout.maximumWidth: messageContainer.bubbleMaxWidth
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        Layout.bottomMargin: Kirigami.Units.largeSpacing
                        Layout.leftMargin: Config.showAvatarInTimeline ? Kirigami.Units.largeSpacing : 0
                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: openMessageContext(author, model.message, eventId, toolTip, eventType, model.formattedBody)
                        }
                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onLongPressed: openMessageContext(author, model.message, eventId, toolTip, eventType, model.formattedBody)
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "notice"
                delegate: TimelineContainer {
                    id: noticeContainer
                    width: messageListView.width
                    isLoaded: timelineDelegateChooser.delegateLoaded
                    onReplyClicked: goToEvent(eventID)

                    innerObject: TextDelegate {
                        Layout.fillWidth: !Config.showAvatarInTimeline
                        Layout.maximumWidth: noticeContainer.bubbleMaxWidth
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        Layout.leftMargin: Config.showAvatarInTimeline ? Kirigami.Units.largeSpacing : 0
                        Layout.bottomMargin: Kirigami.Units.largeSpacing * 2
                    }
                }
            }

            DelegateChoice {
                roleValue: "image"
                delegate: TimelineContainer {
                    id: imageContainer
                    isLoaded: timelineDelegateChooser.delegateLoaded
                    width: messageListView.width
                    onReplyClicked: goToEvent(eventID)
                    hoverComponent: hoverActions

                    innerObject: ImageDelegate {
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 15
                        Layout.maximumWidth: imageContainer.bubbleMaxWidth
                        Layout.bottomMargin: Kirigami.Units.largeSpacing
                        Layout.preferredHeight: info.h / info.w * width
                        Layout.maximumHeight: Kirigami.Units.gridUnit * 20
                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: openFileContext(author, model.display, eventId, toolTip, progressInfo, parent)
                        }
                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onLongPressed: openFileContext(author, model.display, eventId, toolTip, progressInfo, parent)
                            onTapped: {
                                fullScreenImage.createObject(parent, {"filename": eventId, "localPath": currentRoom.urlToDownload(eventId)}).showFullScreen()
                            }
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "sticker"
                delegate: TimelineContainer {
                    isLoaded: timelineDelegateChooser.delegateLoaded
                    width: messageListView.width
                    onReplyClicked: goToEvent(eventID)
                    hoverComponent: hoverActions
                    cardBackground: false

                    innerObject: ImageDelegate {
                        readonly: true
                        Layout.maximumWidth: Kirigami.Units.gridUnit * 10
                        Layout.minimumWidth: Kirigami.Units.gridUnit * 10
                        Layout.preferredHeight: info.h / info.w * width
                    }
                }
            }

            DelegateChoice {
                roleValue: "audio"
                delegate: TimelineContainer {
                    id: audioContainer
                    width: messageListView.width
                    isLoaded: timelineDelegateChooser.delegateLoaded
                    onReplyClicked: goToEvent(eventID)
                    hoverComponent: hoverActions

                    innerObject: AudioDelegate {
                        Layout.fillWidth: true
                        Layout.maximumWidth: audioContainer.bubbleMaxWidth
                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: openFileContext(author, model.display, eventId, toolTip, progressInfo, parent)
                        }
                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onLongPressed: openFileContext(author, model.display, eventId, toolTip, progressInfo, parent)
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "video"
                delegate: TimelineContainer {
                    id: videoContainer
                    width: messageListView.width
                    isLoaded: timelineDelegateChooser.delegateLoaded
                    onReplyClicked: goToEvent(eventID)
                    hoverComponent: hoverActions

                    innerObject: VideoDelegate {
                        Layout.fillWidth: true
                        Layout.maximumWidth: videoContainer.bubbleMaxWidth
                        Layout.preferredHeight: content.info.h / content.info.w * width
                        Layout.maximumHeight: Kirigami.Units.gridUnit * 15
                        Layout.minimumHeight: Kirigami.Units.gridUnit * 5

                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: openFileContext(author, model.display, eventId, toolTip, progressInfo, parent)
                        }
                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onLongPressed: openFileContext(author, model.display, eventId, toolTip, progressInfo, parent)
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "file"
                delegate: TimelineContainer {
                    id: fileContainer
                    width: messageListView.width
                    isLoaded: timelineDelegateChooser.delegateLoaded
                    onReplyClicked: goToEvent(eventID)

                    innerObject: FileDelegate {
                        Layout.fillWidth: true
                        Layout.maximumWidth: fileContainer.bubbleMaxWidth
                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: openFileContext(author, model.display, eventId, toolTip, progressInfo, parent)
                        }
                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onLongPressed: openFileContext(author, model.display, eventId, toolTip, progressInfo, parent)
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "readMarker"
                delegate: QQC2.ItemDelegate {
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
                        border.width: Kirigami.Units.devicePixelRatio
                    }

                    Timer {
                        id: makeMeDisapearTimer
                        interval: Kirigami.Units.humanMoment * 2
                        onTriggered: currentRoom.markAllMessagesAsRead();
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
                            currentRoom.markAllMessagesAsRead();
                            return;
                        }

                        // mark the last visible index 
                        const lastVisibleIdx = lastVisibleIndex();

                        if (lastVisibleIdx < index) {
                            currentRoom.readMarkerEventId = sortedMessageEventModel.data(sortedMessageEventModel.index(lastVisibleIdx, 0), MessageEventModel.EventIdRole)
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "other"
                delegate: Rectangle {
                    height: 10
                    width: ListView.view.width
                    color: "red"
                }
            }
        }

        QQC2.RoundButton {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: Kirigami.Units.largeSpacing
            anchors.rightMargin: Kirigami.Units.largeSpacing
            implicitWidth: Kirigami.Units.gridUnit * 2
            implicitHeight: Kirigami.Units.gridUnit * 2

            id: goReadMarkerFab

            visible: currentRoom && currentRoom.hasUnreadMessages && currentRoom.readMarkerLoaded
            action: Kirigami.Action {
                onTriggered: {
                    goToEvent(currentRoom.readMarkerEventId)
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

            visible: !messageListView.atYEnd
            action: Kirigami.Action {
                onTriggered: {
                    goToLastMessage();
                }
                icon.name: "go-down"
            }

            QQC2.ToolTip {
                text: i18n("Jump to latest message")
            }
        }

        Component.onCompleted: {
            if (currentRoom) {
                if (currentRoom.timelineSize < 20) {
                    currentRoom.getPreviousContent(50);
                }
            }

            positionViewAtBeginning();
        }

        DropArea {
            id: dropAreaFile
            anchors.fill: parent
            onDropped: ChatBoxHelper.attachmentPath = drop.urls[0]
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
            id: messageSourceSheet

            MessageSourceSheet {}
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

        header: TypingPane {
            id: typingPane
            visible: !loadingIndicator.visible && currentRoom && currentRoom.usersTyping.length > 0
            typingNotification: visible ? i18ncp("Message displayed when some users are typing", "%2 is typing", "%2 are typing", currentRoom.usersTyping.length, currentRoom.usersTyping.map(user => user.displayName).join(", ")) : ""
            width: parent.width
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
        headerPositioning: ListView.OverlayHeader

    }


    footer: ChatBox {
        id: chatBox
        visible: !invitation.visible && !(messageListView.count === 0 && !currentRoom.allHistoryLoaded)
        onMessageSent: {
            if (!messageListView.atYEnd) {
                goToLastMessage();
            }
        }
        onEditLastUserMessage: {
            const targetMessage = messageEventModel.getLastLocalUserMessageEventId();
            if (targetMessage) {
                ChatBoxHelper.edit(targetMessage["body"], targetMessage["body"], targetMessage["event_id"]);
                chatBox.focusInputField();
            }
        }
        onReplyPreviousUserMessage: {
            const replyResponse = messageEventModel.getLatestMessageFromIndex(0);
            if (replyResponse && replyResponse["event_id"]) {
                ChatBoxHelper.replyToMessage(replyResponse["event_id"], replyResponse["event"], replyResponse["sender_id"]);
            }
        }
    }

    background: FancyEffectsContainer {
        id: fancyEffectsContainer

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
            target: chatBox
            function onFancyEffectsReasonFound(fancyEffect) {
                fancyEffectsContainer.processFancyEffectsReason(fancyEffect)
            }
        }
    }


    function warning(title, message) {
        page.header.contentItem.text = `${title}<br />${message}`;
        page.header.contentItem.type =  Kirigami.MessageType.Warning;
        page.header.contentItem.visible = true;
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

    function goToEvent(eventID) {
        messageListView.positionViewAtIndex(eventToIndex(eventID), ListView.Contain)
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
        while(index === -1 && i < 100) {
            index = messageListView.indexAt(center, messageListView.y + messageListView.contentY + i);
            i++;
        }
        return index
    }

    function lastVisibleIndex() {
        let center = messageListView.x + messageListView.width / 2;
        let index = -1
        let i = 0
        while(index === -1 && i < 100) {
            index = messageListView.indexAt(center, messageListView.y + messageListView.contentY + messageListView.height - i);
            i++
        }
        return index;
    }

    /// Open message context dialog for file and videos
    function openFileContext(author, message, eventId, source, progressInfo, file) {
        const contextMenu = fileDelegateContextMenu.createObject(page, {
            author: author,
            message: message,
            eventId: eventId,
            source: source,
            file: file,
            progressInfo: progressInfo,
        });
        contextMenu.open();
    }

    /// Open context menu for normal message
    function openMessageContext(author, message, eventId, source, eventType, formattedBody) {
        console.log("message", message)
        const contextMenu = messageDelegateContextMenu.createObject(page, {
            author: author,
            message: message,
            eventId: eventId,
            formattedBody: formattedBody,
            source: source,
            eventType: eventType
        });
        contextMenu.open();
    }
}
