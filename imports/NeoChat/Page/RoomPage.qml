/**
 * SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.15
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12
import Qt.labs.qmlmodels 1.0
import Qt.labs.platform 1.0 as Platform
import QtQuick.Controls.Material 2.12

import org.kde.kirigami 2.13 as Kirigami
import org.kde.kitemmodels 1.0
import org.kde.neochat 1.0

import NeoChat.Component 1.0
import NeoChat.Component.ChatBox 1.0
import NeoChat.Component.Timeline 1.0
import NeoChat.Dialog 1.0
import NeoChat.Menu.Timeline 1.0

Kirigami.ScrollablePage {
    id: page

    required property var currentRoom

    title: currentRoom.displayName

    signal switchRoomUp()
    signal switchRoomDown()

    Connections {
        target: Controller.activeConnection
        function onJoinedRoom(room) {
            if(room.id === invitation.id) {
                roomManager.enterRoom(room);
            }
        }
    }

    Connections {
        target: roomManager.actionsHandler
        onShowMessage: {
            page.header.contentItem.text = message;
            page.header.contentItem.type = messageType === ActionsHandler.Error ? Kirigami.MessageType.Error : Kirigami.MessageType.Information;
            page.header.contentItem.visible = true;
        }

        onHideMessage: page.header.contentItem.visible = false
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

                onClicked: {
                    page.currentRoom.forget()
                    roomManager.getBack();
                }
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
        anchors.centerIn: parent
        visible: page.currentRoom === null || (messageListView.count === 0 && !page.currentRoom.allHistoryLoaded && !page.currentRoom.isInvite)
        QQC2.BusyIndicator {
            running: true
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
            chatBox.focus();
            return;
        }
    }

    // hover actions on a delegate, activated in TimelineContainer.qml
    Item {
        id: hoverActions
        property bool showEdit
        property bool hovered: false

        visible: (hovered || hoverHandler.hovered) && !Kirigami.Settings.isMobile

        property var editClicked
        property var replyClicked
        property var reacted

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
                visible: actions.hovered
                icon.name: "preferences-desktop-emoticons"
                onClicked: emojiDialog.open();
                EmojiDialog {
                    id: emojiDialog
                    onReact: hoverActions.reacted(emoji)
                }
            }
            QQC2.Button {
                QQC2.ToolTip.text: i18n("Edit")
                QQC2.ToolTip.visible: hovered
                visible: hoverActions.showEdit
                icon.name: "document-edit"
                onClicked: hoverActions.editClicked()
            }
            QQC2.Button {
                QQC2.ToolTip.text: i18n("Reply")
                QQC2.ToolTip.visible: hovered
                visible: actions.hovered
                icon.name: "mail-replied-symbolic"
                onClicked: hoverActions.replyClicked()
            }
        }
    }

    ListView {
        id: messageListView
        pixelAligned: true
        visible: !invitation.visible

        readonly property int largestVisibleIndex: count > 0 ? indexAt(contentX + (width / 2), contentY + height - 1) : -1
        readonly property bool noNeedMoreContent: !currentRoom || currentRoom.eventsHistoryJob || currentRoom.allHistoryLoaded
        readonly property bool isLoaded: page.width * page.height > 10

        spacing: Kirigami.Units.smallSpacing
        reuseItems: true

        verticalLayoutDirection: ListView.BottomToTop
        highlightMoveDuration: 500

        model: !isLoaded ? undefined : sortedMessageEventModel


        onContentYChanged: updateReadMarker()
        onCountChanged: updateReadMarker()

        function updateReadMarker() {
            if(!noNeedMoreContent && contentY  - 5000 < originY)
                currentRoom.getPreviousContent(20);
            const index = currentRoom.readMarkerEventId ? eventToIndex(currentRoom.readMarkerEventId) : 0
            if(index === -1) {
                return
            }
            if(firstVisibleIndex() === -1 || lastVisibleIndex() === -1) {
                return
            }
            if(index < firstVisibleIndex() && index > lastVisibleIndex()) {
                currentRoom.readMarkerEventId = sortedMessageEventModel.data(sortedMessageEventModel.index(lastVisibleIndex(), 0), MessageEventModel.EventIdRole)
            }
        }

        MessageEventModel {
            id: messageEventModel

            room: currentRoom
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

                            chatBox.attach(path)
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
                        var localPath = Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/screenshots/" + (new Date()).getTime() + ".png"
                        if (!Clipboard.saveImage(localPath)) return
                        chatBox.attach(localPath)
                        attachDialog.close()
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

        populate: Transition {
            NumberAnimation {
                property: "opacity"; from: 0; to: 1
                duration: Kirigami.Units.shortDuration
            }
        }

        add: Transition {
            NumberAnimation {
                property: "opacity"; from: 0; to: 1
                duration: Kirigami.Units.shortDuration
            }
        }

        move: Transition {
            NumberAnimation {
                property: "y"
                duration: Kirigami.Units.shortDuration
            }
            NumberAnimation {
                property: "opacity"; to: 1
            }
        }

        displaced: Transition {
            NumberAnimation {
                property: "y"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutQuad
            }
            NumberAnimation {
                property: "opacity"; to: 1
            }
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
                    width: messageListView.width - Kirigami.Units.largeSpacing
                    isLoaded: timelineDelegateChooser.delegateLoaded
                    isEmote: true
                    mouseArea: MouseArea {
                        acceptedButtons: Qt.RightButton
                        anchors.fill: parent
                        onClicked: openMessageContext(author, display, eventId, toolTip);
                    }
                    onReplyClicked: goToEvent(eventID)
                    onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);
                    onEdit: chatBox.edit(message, formattedBody, eventId)

                    hoverComponent: hoverActions

                    innerObject: TextDelegate {
                        isEmote: true
                        Layout.fillWidth: true
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        Layout.leftMargin: Kirigami.Units.largeSpacing
                    }
                }
            }

            DelegateChoice {
                roleValue: "message"
                delegate: TimelineContainer {
                    id: timeline
                    width: messageListView.width - Kirigami.Units.largeSpacing

                    isLoaded: timelineDelegateChooser.delegateLoaded
                    onReplyClicked: goToEvent(eventID)
                    onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);
                    onEdit: chatBox.edit(message, formattedBody, eventId)

                    hoverComponent: hoverActions

                    innerObject: TextDelegate {
                        Layout.fillWidth: true
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        Layout.leftMargin: Kirigami.Units.largeSpacing
                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: openMessageContext(author, display, eventId, toolTip)
                        }
                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onLongPressed: openMessageContext(author, display, eventId, toolTip)
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "notice"
                delegate: TimelineContainer {
                    width: messageListView.width - Kirigami.Units.largeSpacing
                    isLoaded: timelineDelegateChooser.delegateLoaded
                    onReplyClicked: goToEvent(eventID)
                    onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);
                    onEdit: chatBox.edit(message, formattedBody, eventId)

                    hoverComponent: hoverActions
                    innerObject: TextDelegate {
                        Layout.fillWidth: true
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        Layout.leftMargin: Kirigami.Units.largeSpacing
                    }
                }
            }

            DelegateChoice {
                roleValue: "image"
                delegate: TimelineContainer {
                    width: messageListView.width - Kirigami.Units.largeSpacing

                    isLoaded: timelineDelegateChooser.delegateLoaded
                    onReplyClicked: goToEvent(eventID)
                    onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);

                    hoverComponent: hoverActions

                    innerObject: ImageDelegate {
                        Layout.minimumWidth: Kirigami.Units.gridUnit * 10
                        Layout.fillWidth: true
                        Layout.preferredHeight: info.h / info.w * width
                        Layout.maximumHeight: Kirigami.Units.gridUnit * 15
                        Layout.maximumWidth: Kirigami.Units.gridUnit * 30
                    }
                }
            }

            DelegateChoice {
                roleValue: "sticker"
                delegate: TimelineContainer {
                    width: messageListView.width - Kirigami.Units.largeSpacing

                    isLoaded: timelineDelegateChooser.delegateLoaded
                    onReplyClicked: goToEvent(eventID)
                    onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);

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
                    width: messageListView.width - Kirigami.Units.largeSpacing

                    hoverComponent: hoverActions

                    isLoaded: timelineDelegateChooser.delegateLoaded
                    onReplyClicked: goToEvent(eventID)
                    onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);

                    innerObject: AudioDelegate {
                        Layout.fillWidth: true
                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: openFileContext(author, display, eventId, toolTip, progressInfo, parent)
                        }
                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onLongPressed: openFileContext(author, display, eventId, toolTip, progressInfo, parent)
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "video"
                delegate: TimelineContainer {
                    width: messageListView.width - Kirigami.Units.largeSpacing

                    hoverComponent: hoverActions

                    isLoaded: timelineDelegateChooser.delegateLoaded
                    onReplyClicked: goToEvent(eventID)
                    onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);

                    innerObject: VideoDelegate {
                        Layout.fillWidth: true
                        Layout.minimumWidth: Kirigami.Units.gridUnit * 10
                        Layout.maximumWidth: Kirigami.Units.gridUnit * 30
                        Layout.preferredHeight: content.info.h / content.info.w * width
                        Layout.maximumHeight: Kirigami.Units.gridUnit * 15
                        Layout.minimumHeight: Kirigami.Units.gridUnit * 5

                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: openFileContext(author, display, eventId, toolTip, progressInfo, parent)
                        }
                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onLongPressed: openFileContext(author, display, eventId, toolTip, progressInfo, parent)
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "file"
                delegate: TimelineContainer {
                    width: messageListView.width - Kirigami.Units.largeSpacing

                    hoverComponent: hoverActions

                    isLoaded: timelineDelegateChooser.delegateLoaded
                    onReplyClicked: goToEvent(eventID)
                    onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);

                    innerObject: FileDelegate {
                        Layout.fillWidth: true
                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: openFileContext(author, display, eventId, toolTip, progressInfo, parent)
                        }
                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onLongPressed: openFileContext(author, display, eventId, toolTip, progressInfo, parent)
                        }
                    }
                }
            }


            DelegateChoice {
                roleValue: "other"
                delegate: Item {}
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

            visible: currentRoom && currentRoom.hasUnreadMessages && currentRoom.readMarkerLoaded || !messageListView.atYEnd
            action: Kirigami.Action {
                onTriggered: {
                    if (currentRoom && currentRoom.hasUnreadMessages) {
                        goToEvent(currentRoom.readMarkerEventId)
                    } else {
                        currentRoom.markAllMessagesAsRead()
                        messageListView.positionViewAtBeginning()
                    }
                }
                icon.name: currentRoom && currentRoom.hasUnreadMessages ? "go-up" : "go-down"
            }

            QQC2.ToolTip {
                text: currentRoom && currentRoom.hasUnreadMessages ? i18n("Jump to first unread message") : i18n("Jump to latest message")
            }
        }

        header: RowLayout {
            id: typingNotification

            visible: currentRoom && currentRoom.usersTyping.length > 0
            height: visible ? implicitHeight: 0
            spacing: Kirigami.Units.largeSpacing

            QQC2.BusyIndicator {
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
            }
            QQC2.Label {
                text: visible ? i18ncp("Message displayed when some users are typing", "%2 is typing", "%2 are typing", currentRoom.usersTyping.length, currentRoom.usersTyping.map(user => user.displayName).join(", ")) : ""
            }
        }


        Component.onCompleted: {
            updateReadMarker()
            if (currentRoom) {
                if (currentRoom.timelineSize < 20)
                    currentRoom.getPreviousContent(50)
            }

            positionViewAtBeginning();
        }

        DropArea {
            id: dropAreaFile
            anchors.fill: parent
            onDropped: chatBox.attach(drop.urls[0])
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
            id: openFolderDialog

            OpenFolderDialog {}
        }

        Component {
            id: fileDelegateContextMenu

            FileDelegateContextMenu {}
        }
    }

    footer: ChatBox {
        id: chatBox
        visible: !invitation.visible && !(messageListView.count === 0 && !currentRoom.allHistoryLoaded)
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
            onFancyEffectsReasonFound: {
                fancyEffectsContainer.processFancyEffectsReason(fancyEffect)
            }
        }

        Connections {
            enabled: Config.showFancyEffects
            target: chatBox
            onFancyEffectsReasonFound: {
                fancyEffectsContainer.processFancyEffectsReason(fancyEffect)
            }
        }
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
    function openFileContext(author, message, eventId, toolTip, progressInfo, file) {
        const contextMenu = fileDelegateContextMenu.createObject(page, {
            'author': author,
            'message': message,
            'eventId': eventId,
        });
        contextMenu.downloadAndOpen.connect(function() {
            if (file.downloaded) {
                if (!Qt.openUrlExternally(progressInfo.localPath)) {
                    Qt.openUrlExternally(progressInfo.localDir);
                }
            } else {
                file.onDownloadedChanged.connect(function() {
                    if (!Qt.openUrlExternally(progressInfo.localPath)) {
                        Qt.openUrlExternally(progressInfo.localDir);
                    }
                });
                currentRoom.downloadFile(eventId, Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId))
            }
        });
        contextMenu.saveFileAs.connect(function() {
            const folderDialog = openFolderDialog.createObject(ApplicationWindow.overlay)

            folderDialog.chosen.connect(function(path) {
                if (!path) {
                    return;
                }

                currentRoom.downloadFile(eventId, path + "/" + currentRoom.fileNameToDownload(eventId))
            })

            folderDialog.open()
        })
        contextMenu.viewSource.connect(function() {
            messageSourceSheet.createObject(page, {
                'sourceText': toolTip,
            }).open();
        });
        contextMenu.reply.connect(function(replyUser, replyContent) {
            replyToMessage(replyUser, replyContent, eventId);
        })
        contextMenu.remove.connect(function() {
            currentRoom.redactEvent(eventId);
        })
        contextMenu.open();
    }

    /// Open context menu for normal message
    function openMessageContext(author, message, eventId, toolTip) {
        const contextMenu = messageDelegateContextMenu.createObject(page, {
            'author': author,
            'message': message,
            'eventId': eventId,
        });
        contextMenu.viewSource.connect(function() {
            messageSourceSheet.createObject(page, {
                'sourceText': toolTip,
            }).open();
        });
        contextMenu.reply.connect(function(replyUser, replyContent) {
            replyToMessage(replyUser, replyContent, eventId);
        })
        contextMenu.remove.connect(function() {
            currentRoom.redactEvent(eventId);
        })
        contextMenu.open();
    }

    function replyToMessage(replyUser, replyContent, eventId) {
        chatBox.editEventId = "";
        chatBox.replyUser = replyUser;
        chatBox.replyEventId = eventId;
        chatBox.replyContent = replyContent;
        chatBox.focusInputField();
    }
}
