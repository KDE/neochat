// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.config as KConfig

import org.kde.neochat
import org.kde.neochat.login
import org.kde.neochat.settings

Kirigami.ApplicationWindow {
    id: root

    property NeoChatConnection connection: Controller.activeConnection
    readonly property HoverLinkIndicator hoverLinkIndicator: linkIndicator

    readonly property QuickSwitcher quickSwitcher: QuickSwitcher {
        connection: root.connection
        window: root
    }

    title: {
        if (NeoChatConfig.windowTitleFocus) {
            return activeFocusItem + " " + (activeFocusItem ? activeFocusItem.Accessible.name : "");
        } else if (RoomManager.currentRoom) {
            return RoomManager.currentRoom.displayName;
        } else {
            return Application.displayName;
        }
    }

    minimumWidth: Kirigami.Units.gridUnit * 20
    minimumHeight: Kirigami.Units.gridUnit * 15

    visible: false // Will be overridden in Component.onCompleted
    wideScreen: width > Kirigami.Units.gridUnit * 65

    property Item states : Item {
        id: states
        states: [
            State { // No Connection -> WelcomePage
                name: "noConnection"
                when: !Controller.activeConnection
                PropertyChanges {
                    root.pageStack.items: [welcomePage]
                }
            },
            State { // Connection Loading -> LoadingPage
                name: "loading"
                when: Controller.activeConnection && !Controller.activeConnection.initialSyncDone
                PropertyChanges {
                    root.pageStack.items: [loadingPage]
                }
            },
            State { // Loaded but no room -> only RoomListPage
                name: "noRoom"
                when: Controller.activeConnection && Controller.activeConnection.initialSyncDone && !RoomManager.currentRoom
                PropertyChanges {
                    root.pageStack.items: [roomListPage]
                }
            },
            State { // Loaded and room -> RoomListPage + RoomPage
                name: "room"
                when: Controller.activeConnection && Controller.activeConnection.initialSyncDone && RoomManager.currentRoom
                PropertyChanges {
                    root.pageStack.items: [roomListPage, roomPage]
                }
            }
        ]
    }

    pageStack {
        columnView.columnResizeMode: pageStack.wideMode ? Kirigami.ColumnView.DynamicColumns : Kirigami.ColumnView.SingleColumn
        globalToolBar.canContainHandles: true
        globalToolBar {
            style: Kirigami.ApplicationHeaderStyle.ToolBar
            showNavigationButtons: pageStack.currentIndex > 0 || pageStack.layers.depth > 1 ? Kirigami.ApplicationHeaderStyle.ShowBackButton : 0
        }
    }

    onConnectionChanged: {
        CustomEmojiModel.connection = root.connection;
        SpaceHierarchyCache.connection = root.connection;
        NeoChatSettingsView.connection = root.connection;
        if (ShareHandler.text && root.connection) {
            root.handleShare();
        }
    }

    Connections {
        target: root.quitAction
        function onTriggered() {
            Qt.quit();
        }
    }

    Loader {
        active: Kirigami.Settings.hasPlatformMenuBar && !Kirigami.Settings.isMobile
        sourceComponent: GlobalMenu {
            connection: root.connection
            appWindow: root
        }
    }

    KConfig.WindowStateSaver {
        configGroupName: "MainWindow"
    }

    Connections {
        target: RoomManager

        function onAskJoinRoom(room) {
            (Qt.createComponent("org.kde.neochat", "JoinRoomDialog").createObject(root, {
                room: room,
                connection: root.connection
            }) as JoinRoomDialog).open();
        }

        function onShowUserDetail(user, room) {
            root.showUserDetail(user, room);
        }

        function onAskDirectChatConfirmation(user) {
            (Qt.createComponent("org.kde.neochat", "AskDirectChatConfirmation").createObject(this, {
                user: user
            }) as AskDirectChatConfirmation).open();
        }

        function onExternalUrl(url) {
            (Qt.createComponent("org.kde.neochat", "ConfirmUrlDialog").createObject(this, {
                link: url
            }) as ConfirmUrlDialog).open();
        }

        function onConnectionChanged(): void {
            if (connection) {
                RoomManager.loadInitialRoom();
            }
        }
    }

    function openRoomDrawer() {
        const page = pageStack.push(Qt.createComponent('org.kde.neochat', 'RoomDrawerPage'), {
            connection: root.connection,
            room: RoomManager.currentRoom,
            userListModel: RoomManager.userListModel,
            mediaMessageFilterModel: RoomManager.mediaMessageFilterModel
        });
        page.resolveResource.connect((idOrUri, action) => RoomManager.resolveResource(idOrUri, action))
    }

    contextDrawer: RoomDrawer {
        room: RoomManager.currentRoom
        connection: root.connection
        userListModel: RoomManager.userListModel
        mediaMessageFilterModel: RoomManager.mediaMessageFilterModel

        onResolveResource: (idOrUri, action) => RoomManager.resolveResource(idOrUri, action)

        roomDrawerWidth: NeoChatConfig.roomDrawerWidth
        onRoomDrawerWidthChanged: {
            NeoChatConfig.roomDrawerWidth = actualWidth;
            NeoChatConfig.save();
        }

        handleClosedIcon.source: "documentinfo-symbolic"
        handleClosedToolTip: i18nc("@action:button", "Show Room Information")

        // Default icon is fine, only need to override the tooltip text
        handleOpenToolTip: i18nc("@action:button", "Close Room Information Drawer")

        modal: (!root.wideScreen || !enabled)
        onEnabledChanged: drawerOpen = enabled && !modal
        enabled: RoomManager.hasOpenRoom && root.pageStack.layers.depth < 2 && root.pageStack.depth < 3 && (root.pageStack.visibleItems.length > 1 || root.pageStack.currentIndex > 0) && !Kirigami.Settings.isMobile && root.pageStack.wideMode
        handleVisible: enabled
    }

    Component.onCompleted: {
        CustomEmojiModel.connection = root.connection;
        SpaceHierarchyCache.connection = root.connection;
        RoomSettingsView.window = root;
        NeoChatSettingsView.window = root;
        NeoChatSettingsView.connection = root.connection;
        WindowController.setBlur(pageStack, NeoChatConfig.blur && !NeoChatConfig.compactLayout);
        if (ShareHandler.text && root.connection) {
            root.handleShare()
        }
        const hasSystemTray = Controller.supportSystemTray && NeoChatConfig.systemTray;
        if (Kirigami.Settings.isMobile || !(hasSystemTray && NeoChatConfig.minimizeToSystemTrayOnStartup)) {
            visible = true;
        }
    }
    Connections {
        target: NeoChatConfig
        function onBlurChanged() {
            WindowController.setBlur(root.pageStack, NeoChatConfig.blur && !NeoChatConfig.compactLayout);
        }
        function onCompactLayoutChanged() {
            WindowController.setBlur(root.pageStack, NeoChatConfig.blur && !NeoChatConfig.compactLayout);
        }
    }

    // blur effect
    color: NeoChatConfig.blur && !NeoChatConfig.compactLayout ? "transparent" : Kirigami.Theme.backgroundColor

    // we need to apply the translucency effect separately on top of the color
    background: Rectangle {
        color: NeoChatConfig.blur && !NeoChatConfig.compactLayout ? Qt.rgba(Kirigami.Theme.backgroundColor.r, Kirigami.Theme.backgroundColor.g, Kirigami.Theme.backgroundColor.b, 1 - NeoChatConfig.transparency) : "transparent"
    }

    readonly property WelcomePage welcomePage: WelcomePage {
        showExisting: true

        parent: states
        objectName: "WelcomePage"
    }

    readonly property Kirigami.Page loadingPage : Kirigami.Page {
        title: i18nc("@title", "Loading")
        globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None

        parent: states
        Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
        }
    }

    readonly property RoomPage roomPage : RoomPage {
        parent: states
    }

    readonly property RoomListPage roomListPage : RoomListPage {
        id: roomList

        parent: states
        onSearch: root.quickSwitcher.open()

        connection: root.connection

        Shortcut {
            sequences: ["Ctrl+PgUp", "Ctrl+Backtab", "Alt+Up"]
            onActivated: {
                roomList.goToPreviousRoom();
            }
        }

        Shortcut {
            sequences: ["Ctrl+PgDown", "Ctrl+Tab", "Alt+Down"]
            onActivated: {
                roomList.goToNextRoom();
            }
        }

        Shortcut {
            sequence: "Alt+Shift+Up"
            onActivated: {
                roomList.goToPreviousUnreadRoom();
            }
        }

        Shortcut {
            sequence: "Alt+Shift+Down"
            onActivated: {
                roomList.goToNextUnreadRoom();
            }
        }
    }

    Connections {
        target: Controller

        function onErrorOccured(error) {
            root.showPassiveNotification(error, "short");
        }
    }

    Connections {
        target: root.connection

        function onNewKeyVerificationSession(session) {
            root.pageStack.pushDialogLayer(Qt.createComponent("org.kde.neochat", "KeyVerificationDialog"), {
                session: session,
                connection: root.connection,
            }, {
                title: i18nc("@title:window", "Session Verification")
            });
        }
        function onUserConsentRequired(url) {
            (Qt.createComponent("org.kde.neochat", "ConsentDialog").createObject(this, {
                url: url
            }) as ConsentDialog).open();
        }
    }

    HoverLinkIndicator {
        id: linkIndicator

        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            rightMargin: Kirigami.Units.largeSpacing
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+,"
        onActivated: {
            NeoChatSettingsView.open();
        }
    }

    Connections {
        target: ShareHandler
        function onTextChanged(): void {
            if (root.connection && ShareHandler.text.length > 0) {
                root.handleShare();
            }
        }
    }
    function handleShare(): void {
        const dialog = root.pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ChooseRoomDialog'), {
            connection: root.connection
        }, {
            title: i18nc("@title", "Share"),
            width: Kirigami.Units.gridUnit * 25
        })
        dialog.chosen.connect(function(targetRoomIds) {
            let targetRoomId = targetRoomIds[0];
            RoomManager.resolveResource(targetRoomId)
            ShareHandler.room = targetRoomId
        })
    }
    function showUserDetail(user, room) {
        const dialog = Qt.createComponent("org.kde.neochat", "UserDetailDialog").createObject(root, {
            room: room,
            user: user,
            connection: root.connection,
        }) as UserDetailDialog;
        // FIXME: The reason why we don't want the focusedWindowItem for the room null case (aka QR codes) is because it will parent it to the soon-to-be-destroyed window item.
        // But this won't be a problem if we turn it into a Kirigami.Dialog or some other in-scene item, which it really should be.
        if (room != null) {
            dialog.parent = QmlUtils.focusedWindowItem(); // Kirigami Dialogs overwrite the parent, so we need to set it again
        }
        dialog.open();
    }
}
