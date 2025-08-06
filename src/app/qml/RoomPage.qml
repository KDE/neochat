// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.Page {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    readonly property NeoChatRoom currentRoom: RoomManager.currentRoom

    /**
     * @brief The TimelineModel to use.
     *
     * Required so that new events can be requested when the end of the current
     * local timeline is reached.
     *
     * @note For loading a room in a different window, override this with a new
     *       TimelineModel set with the room to be shown.
     *
     * @sa TimelineModel
     */
    property TimelineModel timelineModel: RoomManager.timelineModel

    /**
     * @brief The MessageFilterModel to use.
     *
     * This model has the filtered list of events that should be shown in the timeline.
     *
     * @note For loading a room in a different window, override this with a new
     *       MessageFilterModel with the new TimelineModel as the source model.
     *
     * @sa TimelineModel, MessageFilterModel
     */
    property MessageFilterModel messageFilterModel: RoomManager.messageFilterModel

    /**
     * @brief The MediaMessageFilterModel to use.
     *
     * This model has the filtered list of media events that should be shown in
     * the timeline.
     *
     * @note For loading a room in a different window, override this with a new
     *       MediaMessageFilterModel with the new MessageFilterModel as the source model.
     *
     * @sa TimelineModel, MessageFilterModel
     */
    property MediaMessageFilterModel mediaMessageFilterModel: RoomManager.mediaMessageFilterModel

    title: root.currentRoom ? root.currentRoom.displayName : ""
    focus: true
    padding: 0

    actions: [
        Kirigami.Action {
            visible: Kirigami.Settings.isMobile || !root.Kirigami.PageStack.pageStack.wideMode
            icon.name: "view-right-new"
            onTriggered: (root.QQC2.ApplicationWindow.window as Main).openRoomDrawer()
        }
    ]

    KeyNavigation.left: pageStack.get(0)

    onCurrentRoomChanged: {
        banner.visible = false;
        if (!Kirigami.Settings.isMobile && chatBarLoader.item) {
            chatBarLoader.item.forceActiveFocus();
        }
    }

    Connections {
        target: root.currentRoom.connection
        function onIsOnlineChanged() {
            if (!root.currentRoom.connection.isOnline) {
                banner.text = i18nc("@info:status", "NeoChat is offline. Please check your network connection.");
                banner.visible = true;
                banner.type = Kirigami.MessageType.Error;
            } else {
                banner.visible = false;
            }
        }
    }

    header: Kirigami.InlineMessage {
        id: banner

        showCloseButton: true
        visible: false
        position: Kirigami.InlineMessage.Position.Header
    }

    Loader {
        id: timelineViewLoader
        anchors.fill: parent
        active: root.currentRoom && !root.currentRoom.isInvite && !root.currentRoom.isSpace
        // We need the loader to be active but invisible while the room is loading messages so signals in TimelineView work.
        visible: !root.loading
        sourceComponent: TimelineView {
            id: timelineView
            messageFilterModel: root.messageFilterModel
            compactLayout: NeoChatConfig.compactLayout
            fileDropEnabled: !Controller.isFlatpak
            markReadCondition: NeoChatConfig.markReadCondition
        }
    }

    Loader {
        id: invitationLoader
        active: root.currentRoom && root.currentRoom.isInvite
        anchors.fill: parent
        sourceComponent: InvitationView {
            currentRoom: root.currentRoom
        }
    }

    Loader {
        id: spaceLoader
        active: root.currentRoom && root.currentRoom.isSpace
        anchors.fill: parent
        sourceComponent: SpaceHomePage {
            room: root.currentRoom
        }
    }

    Loader {
        active: !RoomManager.currentRoom
        anchors.centerIn: parent
        sourceComponent: Kirigami.PlaceholderMessage {
            icon.name: "org.kde.neochat"
            text: i18nc("@title", "Welcome to NeoChat")
            explanation: i18nc("@info:usagetip", "Select or join a room to get started")
        }
    }

    background: Rectangle {
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        color: NeoChatConfig.compactLayout ? Kirigami.Theme.backgroundColor : "transparent"
    }

    footer: Loader {
        id: chatBarLoader
        height: active ? item.implicitHeight : 0
        active: timelineViewLoader.active && !root.currentRoom.readOnly
        sourceComponent: ChatBar {
            id: chatBar
            width: parent.width
            currentRoom: root.currentRoom
            connection: root.currentRoom.connection
        }
    }

    Connections {
        target: RoomManager
        function onCurrentRoomChanged() {
            if (root.currentRoom && root.currentRoom.isInvite) {
                Controller.clearInvitationNotification(root.currentRoom.id);
            }
        }

        function onGoToEvent(eventId) {
            (timelineViewLoader.item as TimelineView).goToEvent(eventId);
        }
    }

    Connections {
        target: root.currentRoom.connection
        function onJoinedRoom(room, invited) {
            if (root.currentRoom.id === invited.id) {
                RoomManager.resolveResource(room.id);
            }
        }
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_PageUp) {
            event.accepted = true;
            timelineViewLoader.item.pageUp();
        } else if (event.key === Qt.Key_PageDown) {
            event.accepted = true;
            timelineViewLoader.item.pageDown();
        }
    }

    Connections {
        target: RoomManager

        function onShowMessage(messageType, message) {
            banner.text = message;
            banner.type = messageType;
            banner.visible = true;
        }

        function onShowEventSource(eventId) {
            (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'MessageSourceSheet'), {
                sourceText: root.currentRoom.getEventJsonSource(eventId)
            }, {
                title: i18nc("@title:dialog", "Message Source"),
                width: Kirigami.Units.gridUnit * 25
            });
        }

        function onShowMessageMenu(eventId, author, messageComponentType, plainText, htmlText, selectedText, hoveredLink, isThread) {
            const contextMenu = messageDelegateContextMenu.createObject(root, {
                selectedText: selectedText,
                hoveredLink: hoveredLink,
                author: author,
                eventId: eventId,
                messageComponentType: messageComponentType,
                plainText: plainText,
                htmlText: htmlText,
            });
            contextMenu.popup();
        }

        function onShowFileMenu(eventId, author, messageComponentType, plainText, mimeType, progressInfo, isThread) {
            const contextMenu = fileDelegateContextMenu.createObject(root, {
                author: author,
                eventId: eventId,
                plainText: plainText,
                mimeType: mimeType,
                progressInfo: progressInfo,
                messageComponentType: messageComponentType,
            });
            contextMenu.popup();
        }

        function onShowMaximizedMedia(index) {
            var popup = maximizeComponent.createObject(QQC2.Overlay.overlay, {
                initialIndex: index
            });
            popup.closed.connect(() => {
                timelineViewLoader.item.interactive = true;
                popup.destroy();
            });
            popup.open();
        }

        function onShowMaximizedCode(author, time, codeText, language) {
            let popup = Qt.createComponent('org.kde.neochat', 'CodeMaximizeComponent').createObject(QQC2.Overlay.overlay, {
                author: author,
                time: time,
                codeText: codeText,
                language: language
            }).open();
        }
    }

    Component {
        id: messageDelegateContextMenu
        MessageDelegateContextMenu {
            room: root.currentRoom
            connection: root.currentRoom.connection
        }
    }

    Component {
        id: fileDelegateContextMenu
        FileDelegateContextMenu {
            room: root.currentRoom
            connection: root.currentRoom.connection
        }
    }

    Component {
        id: maximizeComponent
        NeochatMaximizeComponent {
            currentRoom: root.currentRoom
            model: root.mediaMessageFilterModel
            parent: root.QQC2.Overlay.overlay
        }
    }
}
