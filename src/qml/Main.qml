// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.login
import org.kde.neochat.settings

Kirigami.ApplicationWindow {
    id: root

    property NeoChatConnection connection: Controller.activeConnection
    readonly property HoverLinkIndicator hoverLinkIndicator: linkIndicator


    title: Config.windowTitleFocus ? activeFocusItem + " " + (activeFocusItem ? activeFocusItem.Accessible.name : "") : "NeoChat"

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
        sourceComponent: GlobalMenu {
            connection: root.connection
        }
    }

    // This timer allows to batch update the window size change to reduce
    // the io load and also work around the fact that x/y/width/height are
    // changed when loading the page and overwrite the saved geometry from
    // the previous session.
    Timer {
        id: saveWindowGeometryTimer
        interval: 1000
        onTriggered: WindowController.saveGeometry()
    }

    Connections {
        id: saveWindowGeometryConnections
        enabled: false // Disable on startup to avoid writing wrong values if the window is hidden
        target: root

        function onClosing() {
            WindowController.saveGeometry();
        }
        function onWidthChanged() {
            saveWindowGeometryTimer.restart();
        }
        function onHeightChanged() {
            saveWindowGeometryTimer.restart();
        }
        function onXChanged() {
            saveWindowGeometryTimer.restart();
        }
        function onYChanged() {
            saveWindowGeometryTimer.restart();
        }
    }

    QuickSwitcher {
        id: quickSwitcher
        connection: root.connection
    }

    Connections {
        target: RoomManager

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
        property bool drawerUserState: Config.autoRoomInfoDrawer

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
            if (Config.autoRoomInfoDrawer) {
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
        WindowController.setBlur(pageStack, Config.blur && !Config.compactLayout);
        if (ShareHandler.text && root.connection) {
            root.handleShare()
        }
        if (Config.minimizeToSystemTrayOnStartup && !Kirigami.Settings.isMobile && Controller.supportSystemTray && Config.systemTray) {
            restoreWindowGeometryConnections.enabled = true; // To restore window size and position
        } else {
            visible = true;
            saveWindowGeometryConnections.enabled = true;
        }
    }
    Connections {
        target: Config
        function onBlurChanged() {
            WindowController.setBlur(pageStack, Config.blur && !Config.compactLayout);
        }
        function onCompactLayoutChanged() {
            WindowController.setBlur(pageStack, Config.blur && !Config.compactLayout);
        }
    }

    // blur effect
    color: Config.blur && !Config.compactLayout ? "transparent" : Kirigami.Theme.backgroundColor

    // we need to apply the translucency effect separately on top of the color
    background: Rectangle {
        color: Config.blur && !Config.compactLayout ? Qt.rgba(Kirigami.Theme.backgroundColor.r, Kirigami.Theme.backgroundColor.g, Kirigami.Theme.backgroundColor.b, 1 - Config.transparency) : "transparent"
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

        function onErrorOccured(error, detail) {
            showPassiveNotification(detail.length > 0 ? i18n("%1: %2", error, detail) : error);
        }
    }

    Connections {
        id: restoreWindowGeometryConnections
        enabled: false
        target: root

        function onVisibleChanged() {
            if (!visible) {
                return;
            }
            Controller.restoreWindowGeometry(root);
            restoreWindowGeometryConnections.enabled = false; // Only restore window geometry for the first time
            saveWindowGeometryConnections.enabled = true;
        }
    }

    Connections {
        target: root.connection

        function onDirectChatAvailable(directChat) {
            RoomManager.resolveResource(directChat.id);
        }
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

        anchors.bottom: parent.bottom
        anchors.left: parent.left
    }

    Shortcut {
        sequence: "Ctrl+Shift+,"
        onActivated: {
            pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.settings', 'NeoChatSettings'), {
                connection: root.connection
            }, {
                title: i18n("Configure"),
                width: Kirigami.Units.gridUnit * 50,
                height: Kirigami.Units.gridUnit * 42
            });
        }
    }
    Connections {
        target: ShareHandler
        function onTextChanged(): void {
            if (root.connection && ShareHandler.text.length > 0) {
                handleShare();
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
        Qt.createComponent("org.kde.neochat", "UserDetailDialog").createObject(root.QQC2.ApplicationWindow.window, {
            room: room,
            user: user,
            connection: root.connection
        }).open();
    }

    function load() {
        pageStack.replace(roomListComponent);
        RoomManager.loadInitialRoom();
        let roomPage = pageStack.push(Qt.createComponent('org.kde.neochat', 'RoomPage'), {
            connection: root.connection
        });
        roomPage.forceActiveFocus();
    }
}
