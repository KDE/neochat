// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

Kirigami.Page {
    id: root

    /// Not readonly because of the separate window view.
    property NeoChatRoom currentRoom: RoomManager.currentRoom
    property bool loading: !root.currentRoom || (root.currentRoom.timelineSize === 0 && !root.currentRoom.allHistoryLoaded)
    /// Used to determine if scrolling to the bottom should mark the message as unread
    property bool hasScrolledUpBefore: false;

    /// Disable cancel shortcut. Used by the separate window since it provides its own cancel implementation.
    property bool disableCancelShortcut: false

    title: currentRoom.displayName
    focus: true
    padding: 0

    KeyNavigation.left: pageStack.get(0)

    onCurrentRoomChanged: {
        if (!timelineViewLoader.item) {
            return
        }
        applicationWindow().hoverLinkIndicator.text = "";
        timelineViewLoader.item.positionViewAtBeginning();
        hasScrolledUpBefore = false;
        if (!Kirigami.Settings.isMobile && chatBoxLoader.item) {
            chatBoxLoader.item.forceActiveFocus();
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

    actions.main: Kirigami.Action {
        text: i18n("Call")
        icon.name: "call-start"
        visible: Controller.callsSupported && root.currentRoom.joinedCount === 2
        onTriggered: {
            CallManager.startCall(root.currentRoom, true)
        }
    }


    Loader {
        id: timelineViewLoader
        anchors.fill: parent
        active: currentRoom && !currentRoom.isInvite && !root.loading
        sourceComponent: TimelineView {
            id: timelineView
            currentRoom: root.currentRoom
            onFocusChatBox: {
                if (chatBoxLoader.item) {
                    chatBoxLoader.item.forceActiveFocus()
                }
            }
        }
    }

    Loader {
        id: invitationLoader
        active: currentRoom && currentRoom.isInvite
        anchors.centerIn: parent
        sourceComponent: InvitationView {
            currentRoom: root.currentRoom
            anchors.centerIn: parent
        }
    }

    Loader {
        active: root.loading && !invitationLoader.active
        anchors.centerIn: parent
        sourceComponent: Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
        }
    }

    background: Rectangle {
        color: Config.compactLayout ? Kirigami.Theme.backgroundColor : "transparent"
    }

    footer: Loader {
        id: chatBoxLoader
        active: timelineViewLoader.active
        sourceComponent: ChatBox {
            id: chatBox
            width: parent.width
            currentRoom: currentRoom
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
            if(!RoomManager.currentRoom) {
                if(pageStack.lastItem === root) {
                    pageStack.pop()
                }
            } else if (root.currentRoom.isInvite) {
                root.currentRoom.clearInvitationNotification();
            }
        }

        function onWarning(title, message) {
            root.warning(title, message);
        }
    }

    ActionsHandler {
        id: actionsHandler
        room: root.currentRoom
    }

    Shortcut {
        sequence: StandardKey.Cancel
        onActivated: {
            if (!timelineViewLoader.item.atYEnd || currentRoom.hasUnreadMessages) {
                goToLastMessage();
                currentRoom.markAllMessagesAsRead();
            } else {
                applicationWindow().pageStack.get(0).forceActiveFocus();
            }
        }
        enabled: !root.disableCancelShortcut
    }

    Connections {
        target: Controller.activeConnection
        function onJoinedRoom(room, invited) {
            if(root.currentRoom.id === invited.id) {
                RoomManager.enterRoom(room);
            }
        }
    }

    Keys.onPressed: {
        if (!(event.modifiers & Qt.ControlModifier) && event.key < Qt.Key_Escape) {
            event.accepted = true;
            chatBoxLoader.item.insertText(event.text);
            chatBoxLoader.item.forceActiveFocus();
            return;
        } else if (event.key === Qt.Key_PageUp) {
            event.accepted = true;
            timelineViewLoader.item.pageUp()
        } else if (event.key === Qt.Key_PageDown) {
            event.accepted = true;
            timelineViewLoader.item.pageDown()
        }
    }

    Connections {
        target: currentRoom
        function onShowMessage(messageType, message) {
            root.header.contentItem.text = message;
            root.header.contentItem.type = messageType === ActionsHandler.Error ? Kirigami.MessageType.Error : messageType === ActionsHandler.Positive ? Kirigami.MessageType.Positive : Kirigami.MessageType.Information;
            root.header.visible = true;
        }
    }

    function warning(title, message) {
        root.header.contentItem.text = `${title}<br />${message}`;
        root.header.contentItem.type =  Kirigami.MessageType.Warning;
        root.header.visible = true;
    }
}
