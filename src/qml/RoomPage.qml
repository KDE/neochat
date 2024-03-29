// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigamiaddons.labs.components as KirigamiComponents
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels

import org.kde.neochat
import org.kde.neochat.config

Kirigami.Page {
    id: root

    /// Not readonly because of the separate window view.
    property NeoChatRoom currentRoom: RoomManager.currentRoom

    required property NeoChatConnection connection

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

    /**
     * @brief The ActionsHandler object to use.
     */
    property ActionsHandler actionsHandler: ActionsHandler {
        room: root.currentRoom
    }

    property bool loading: !root.currentRoom || (root.currentRoom.timelineSize === 0 && !root.currentRoom.allHistoryLoaded)

    /// Disable cancel shortcut. Used by the separate window since it provides its own cancel implementation.
    property bool disableCancelShortcut: false

    title: root.currentRoom ? root.currentRoom.displayName : ""
    focus: true
    padding: 0

    actions: [
        Kirigami.Action {
            visible: Kirigami.Settings.isMobile || !applicationWindow().pageStack.wideMode
            icon.name: "view-right-new"
            onTriggered: applicationWindow().openRoomDrawer()
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
        target: root.connection
        function onIsOnlineChanged() {
            if (!root.connection.isOnline) {
                banner.text = i18n("NeoChat is offline. Please check your network connection.");
                banner.visible = true;
                banner.type = Kirigami.MessageType.Error;
            } else {
                banner.visible = false;
            }
        }
    }

    header: KirigamiComponents.Banner {
        id: banner

        showCloseButton: true
        visible: false
    }

    Loader {
        id: timelineViewLoader
        anchors.fill: parent
        active: root.currentRoom && !root.currentRoom.isInvite && !root.loading && !root.currentRoom.isSpace
        sourceComponent: TimelineView {
            id: timelineView
            currentRoom: root.currentRoom
            page: root
            timelineModel: root.timelineModel
            messageFilterModel: root.messageFilterModel
            actionsHandler: root.actionsHandler
            onFocusChatBar: {
                if (chatBarLoader.item) {
                    chatBarLoader.item.forceActiveFocus();
                }
            }
        }
    }

    Loader {
        id: invitationLoader
        active: root.currentRoom && root.currentRoom.isInvite
        anchors.centerIn: parent
        sourceComponent: InvitationView {
            currentRoom: root.currentRoom
            anchors.centerIn: parent
        }
    }

    Loader {
        id: spaceLoader
        active: root.currentRoom && root.currentRoom.isSpace
        anchors.fill: parent
        sourceComponent: SpaceHomePage {}
    }

    Loader {
        active: !RoomManager.currentRoom
        anchors.centerIn: parent
        sourceComponent: Kirigami.PlaceholderMessage {
            icon.name: "org.kde.neochat"
            text: i18n("Welcome to NeoChat")
        }
    }

    Loader {
        active: root.loading && !invitationLoader.active && RoomManager.currentRoom && !spaceLoader.active
        anchors.centerIn: parent
        sourceComponent: Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
        }
    }

    background: Rectangle {
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        color: Config.compactLayout ? Kirigami.Theme.backgroundColor : "transparent"
    }

    footer: Loader {
        id: chatBarLoader
        active: timelineViewLoader.active && !root.currentRoom.readOnly
        sourceComponent: ChatBar {
            id: chatBar
            width: parent.width
            currentRoom: root.currentRoom
            connection: root.connection
            actionsHandler: root.actionsHandler
            onMessageSent: {
                if (!timelineViewLoader.item.atYEnd) {
                    timelineViewLoader.item.goToLastMessage();
                }
            }
        }
    }

    Connections {
        target: RoomManager
        function onCurrentRoomChanged() {
            if (root.currentRoom && root.currentRoom.isInvite) {
                root.currentRoom.clearInvitationNotification();
            }
        }

        function onWarning(title, message) {
            root.warning(title, message);
        }

        function onGoToEvent(eventId) {
            (timelineViewLoader.item as TimelineView).goToEvent(eventId);
        }
    }

    Shortcut {
        sequence: StandardKey.Cancel
        onActivated: {
            if (!timelineViewLoader.item.atYEnd || root.currentRoom.hasUnreadMessages) {
                timelineViewLoader.item.goToLastMessage();
                root.currentRoom.markAllMessagesAsRead();
            } else {
                applicationWindow().pageStack.get(0).forceActiveFocus();
            }
        }
        enabled: !root.disableCancelShortcut
    }

    Connections {
        target: root.connection
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
        target: root.currentRoom
        function onShowMessage(messageType, message) {
            banner.text = message;
            banner.type = messageType === ActionsHandler.Error ? Kirigami.MessageType.Error : messageType === ActionsHandler.Positive ? Kirigami.MessageType.Positive : Kirigami.MessageType.Information;
            banner.visible = true;
        }
    }

    function warning(title, message) {
        banner.text = `${title}<br />${message}`;
        banner.type = Kirigami.MessageType.Warning;
        banner.visible = true;
    }

    Connections {
        target: RoomManager

        function onShowEventSource(eventId) {
            applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'MessageSourceSheet.qml'), {
                sourceText: root.currentRoom.getEventJsonSource(eventId)
            }, {
                title: i18n("Message Source"),
                width: Kirigami.Units.gridUnit * 25
            });
        }

        function onShowMessageMenu(eventId, author, messageComponentType, plainText, htmlText, selectedText) {
            const contextMenu = messageDelegateContextMenu.createObject(root, {
                selectedText: selectedText,
                author: author,
                eventId: eventId,
                messageComponentType: messageComponentType,
                plainText: plainText,
                htmlText: htmlText
            });
            contextMenu.open();
        }

        function onShowFileMenu(eventId, author, messageComponentType, plainText, mimeType, progressInfo) {
            const contextMenu = fileDelegateContextMenu.createObject(root, {
                author: author,
                eventId: eventId,
                plainText: plainText,
                mimeType: mimeType,
                progressInfo: progressInfo
            });
            contextMenu.open();
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
    }

    Component {
        id: messageDelegateContextMenu
        MessageDelegateContextMenu {
            connection: root.connection
        }
    }

    Component {
        id: fileDelegateContextMenu
        FileDelegateContextMenu {
            connection: root.connection
        }
    }

    Component {
        id: maximizeComponent
        NeochatMaximizeComponent {
            currentRoom: root.currentRoom
            model: root.mediaMessageFilterModel
        }
    }
}
