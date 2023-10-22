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
     * @brief The MessageEventModel to use.
     *
     * Required so that new events can be requested when the end of the current
     * local timeline is reached.
     *
     * @note For loading a room in a different window, override this with a new
     *       MessageEventModel set with the room to be shown.
     *
     * @sa MessageEventModel
     */
    property MessageEventModel messageEventModel: RoomManager.messageEventModel

    /**
     * @brief The MessageFilterModel to use.
     *
     * This model has the filtered list of events that should be shown in the timeline.
     *
     * @note For loading a room in a different window, override this with a new
     *       MessageFilterModel with the new MessageEventModel as the source model.
     *
     * @sa MessageEventModel, MessageFilterModel
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
     * @sa MessageEventModel, MessageFilterModel
     */
    property MediaMessageFilterModel mediaMessageFilterModel: RoomManager.mediaMessageFilterModel

    property bool loading: !root.currentRoom || (root.currentRoom.timelineSize === 0 && !root.currentRoom.allHistoryLoaded)

    /// Disable cancel shortcut. Used by the separate window since it provides its own cancel implementation.
    property bool disableCancelShortcut: false

    title: root.currentRoom.displayName
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
        if (!Kirigami.Settings.isMobile && chatBoxLoader.item) {
            chatBoxLoader.item.forceActiveFocus();
        }
    }

    Connections {
        target: Controller
        function onIsOnlineChanged() {
            if (!Controller.isOnline) {
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
        active: root.currentRoom && !root.currentRoom.isInvite && !root.loading
        sourceComponent: TimelineView {
            id: timelineView
            currentRoom: root.currentRoom
            messageEventModel: root.messageEventModel
            messageFilterModel: root.messageFilterModel
            onFocusChatBox: {
                if (chatBoxLoader.item) {
                    chatBoxLoader.item.forceActiveFocus()
                }
            }
            connection: root.connection
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
        active: root.loading && !invitationLoader.active
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
        id: chatBoxLoader
        active: timelineViewLoader.active
        sourceComponent: ChatBox {
            id: chatBox
            width: parent.width
            currentRoom: root.currentRoom
            connection: root.connection
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
            if(root.currentRoom.id === invited.id) {
                RoomManager.enterRoom(room);
            }
        }
    }

    Keys.onPressed: event => {
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
            banner.text = message;
            banner.type = messageType === ActionsHandler.Error ? Kirigami.MessageType.Error : messageType === ActionsHandler.Positive ? Kirigami.MessageType.Positive : Kirigami.MessageType.Information;
            banner.visible = true;
        }
    }

    function warning(title, message) {
        banner.text = `${title}<br />${message}`;
        banner.type =  Kirigami.MessageType.Warning;
        banner.visible = true;
    }

    Connections {
        target: RoomManager
        function onShowUserDetail(user) {
            root.showUserDetail(user)
        }

        function onShowEventSource(eventId) {
            applicationWindow().pageStack.pushDialogLayer('qrc:/org/kde/neochat/qml/MessageSourceSheet.qml', {
                sourceText: root.currentRoom.getEventJsonSource(eventId)
            }, {
                title: i18n("Message Source"),
                width: Kirigami.Units.gridUnit * 25
            });
        }

        function onShowMessageMenu(eventId, author, delegateType, plainText, htmlText, selectedText) {
            const contextMenu = messageDelegateContextMenu.createObject(root, {
                selectedText: selectedText,
                author: author,
                eventId: eventId,
                delegateType: delegateType,
                plainText: plainText,
                htmlText: htmlText
            });
            contextMenu.open();
        }

        function onShowFileMenu(eventId, author, delegateType, plainText, mimeType, progressInfo) {
            const contextMenu = fileDelegateContextMenu.createObject(root, {
                author: author,
                eventId: eventId,
                delegateType: delegateType,
                plainText: plainText,
                mimeType: mimeType,
                progressInfo: progressInfo
            });
            contextMenu.open();
        }

        function onShowMaximizedMedia(index) {
            var popup = maximizeComponent.createObject(QQC2.ApplicationWindow.overlay, {
                initialIndex: index
            })
            popup.closed.connect(() => {
                messageListView.interactive = true
                popup.destroy()
            })
            popup.open()
        }
    }

    function showUserDetail(user) {
        userDetailDialog.createObject(QQC2.ApplicationWindow.overlay, {
            room: root.currentRoom,
            user: root.currentRoom.getUser(user.id),
        }).open();
    }

    Component {
        id: userDetailDialog
        UserDetailDialog {}
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
