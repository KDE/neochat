// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.config as KConfig

import org.kde.neochat
import org.kde.neochat.login
import org.kde.neochat.settings

Kirigami.ApplicationWindow {
    id: root

    property NeoChatConnection connection: Controller.activeConnection
    readonly property HoverLinkIndicator hoverLinkIndicator: linkIndicator

    property bool initialized: false

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

    pageStack {
        initialPage: WelcomePage {
            showExisting: true
            onConnectionChosen: root.load()
        }
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
        target: LoginHelper
        function onLoaded() {
            root.load();
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
        sourceComponent: Qt.createComponent("org.kde.neochat", "GlobalMenu")
        onActiveChanged: if (active) {
            item.connection = root.connection;
        }
    }

    KConfig.WindowStateSaver {
        configGroupName: "MainWindow"
    }

    QuickSwitcher {
        id: quickSwitcher
        connection: root.connection
    }

    Connections {
        target: RoomManager

        function onCurrentRoomChanged() {
            if (RoomManager.currentRoom && pageStack.depth <= 1 && root.initialized && Kirigami.Settings.isMobile) {
                let roomPage = pageStack.layers.push(Qt.createComponent('org.kde.neochat', 'RoomPage'), {
                    connection: root.connection
                });
                roomPage.backRequested.connect(event => {
                    RoomManager.clearCurrentRoom();
                });
            }
        }

        function onAskJoinRoom(room) {
            Qt.createComponent("org.kde.neochat", "JoinRoomDialog").createObject(root, {
                room: room,
                connection: root.connection
            }).open();
        }

        function onShowUserDetail(user, room) {
            root.showUserDetail(user, room);
        }

        function goToEvent(event) {
            if (event.length > 0) {
                roomItem.goToEvent(event);
            }
            roomItem.forceActiveFocus();
        }

        function onAskDirectChatConfirmation(user) {
            Qt.createComponent("org.kde.neochat", "AskDirectChatConfirmation").createObject(this, {
                user: user
            }).open();
        }

        function onExternalUrl(url) {
            let dialog = Qt.createComponent("org.kde.neochat", "ConfirmUrlDialog").createObject(this);
            dialog.link = url;
            dialog.open();
        }
    }

    function pushReplaceLayer(page, args) {
        if (pageStack.layers.depth === 2) {
            pageStack.layers.replace(page, args);
        } else {
            pageStack.layers.push(page, args);
        }
    }

    function openRoomDrawer() {
        pageStack.push(Qt.createComponent('org.kde.neochat', 'RoomDrawerPage'), {
            connection: root.connection
        });
    }

    contextDrawer: RoomDrawer {
        id: contextDrawer

        // This is a memory for all user initiated actions on the drawer, i.e. clicking the button
        // It is used to ensure that user choice is remembered when changing pages and expanding and contracting the window width
        property bool drawerUserState: NeoChatConfig.autoRoomInfoDrawer

        connection: root.connection

        handleClosedIcon.source: "documentinfo-symbolic"
        handleClosedToolTip: i18nc("@action:button", "Show Room Information")

        // Default icon is fine, only need to override the tooltip text
        handleOpenToolTip: i18nc("@action:button", "Close Room Information Drawer")

        // Connect to the onClicked function of the RoomDrawer handle button
        Connections {
            target: contextDrawer.handle.children[0]
            function onClicked() {
                contextDrawer.drawerUserState = contextDrawer.drawerOpen;
            }
        }

        modal: (!root.wideScreen || !enabled)
        onEnabledChanged: drawerOpen = enabled && !modal
        onModalChanged: {
            if (NeoChatConfig.autoRoomInfoDrawer) {
                drawerOpen = !modal && drawerUserState;
                dim = false;
            }
        }
        enabled: RoomManager.hasOpenRoom && pageStack.layers.depth < 2 && pageStack.depth < 3 && (pageStack.visibleItems.length > 1 || pageStack.currentIndex > 0) && !Kirigami.Settings.isMobile && root.pageStack.wideMode
        handleVisible: enabled
    }

    Component.onCompleted: {
        CustomEmojiModel.connection = root.connection;
        SpaceHierarchyCache.connection = root.connection;
        RoomSettingsView.window = root;
        NeoChatSettingsView.window = root;
        NeoChatSettingsView.connection = root.connection;
        WindowController.setBlur(pageStack, NeoChatConfig.blur && !NeoChatConfig.compactLayout);
        TextToSpeechWrapper.warmUp();
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
            WindowController.setBlur(pageStack, NeoChatConfig.blur && !NeoChatConfig.compactLayout);
        }
        function onCompactLayoutChanged() {
            WindowController.setBlur(pageStack, NeoChatConfig.blur && !NeoChatConfig.compactLayout);
        }
    }

    // blur effect
    color: NeoChatConfig.blur && !NeoChatConfig.compactLayout ? "transparent" : Kirigami.Theme.backgroundColor

    // we need to apply the translucency effect separately on top of the color
    background: Rectangle {
        color: NeoChatConfig.blur && !NeoChatConfig.compactLayout ? Qt.rgba(Kirigami.Theme.backgroundColor.r, Kirigami.Theme.backgroundColor.g, Kirigami.Theme.backgroundColor.b, 1 - NeoChatConfig.transparency) : "transparent"
    }

    Component {
        id: roomListComponent
        RoomListPage {
            id: roomList

            onSearch: quickSwitcher.open()

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
    }

    Connections {
        target: AccountRegistry
        function onRowsRemoved() {
            if (AccountRegistry.rowCount() === 0) {
                pageStack.clear();
                pageStack.push(Qt.createComponent('org.kde.neochat.login', 'WelcomePage'));
            }
        }
    }

    Connections {
        target: Controller

        function onErrorOccured(error) {
            showPassiveNotification(error, "short");
        }
    }

    Connections {
        target: root.connection

        function onNewKeyVerificationSession(session) {
            root.pageStack.pushDialogLayer(Qt.createComponent("org.kde.neochat", "KeyVerificationDialog"), {
                session: session
            }, {
                title: i18nc("@title:window", "Session Verification")
            });
        }
        function onUserConsentRequired(url) {
            Qt.createComponent("org.kde.neochat", "ConsentDialog").createObject(this, {
                url: url
            }).open();
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
        const dialog = applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ChooseRoomDialog'), {
            connection: root.connection
        }, {
            title: i18nc("@title", "Share"),
            width: Kirigami.Units.gridUnit * 25
        })
        dialog.chosen.connect(function(targetRoomId) {
            RoomManager.resolveResource(targetRoomId)
            ShareHandler.room = targetRoomId
            dialog.closeDialog()
        })
    }
    function showUserDetail(user, room) {
        const dialog = Qt.createComponent("org.kde.neochat", "UserDetailDialog").createObject(root, {
            room: room,
            user: user,
            connection: root.connection,
        });
        dialog.parent = QmlUtils.focusedWindowItem(); // Kirigami Dialogs overwrite the parent, so we need to set it again
        dialog.open();
    }

    function load() {
        pageStack.replace(roomListComponent);
        RoomManager.loadInitialRoom();

        if (!Kirigami.Settings.isMobile) {
            let roomPage = pageStack.push(Qt.createComponent('org.kde.neochat', 'RoomPage'), {
                connection: root.connection
            });
            roomPage.forceActiveFocus();
        }

        initialized = true;
    }
}
