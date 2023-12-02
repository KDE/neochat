// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.config
import org.kde.neochat.accounts

Kirigami.ApplicationWindow {
    id: root

    property int columnWidth: Kirigami.Units.gridUnit * 13

    property RoomListPage roomListPage
    property bool roomListLoaded: false

    property RoomPage roomPage
    property SpaceHomePage spaceHomePage

    property NeoChatConnection connection: Controller.activeConnection

    minimumWidth: Kirigami.Units.gridUnit * 20
    minimumHeight: Kirigami.Units.gridUnit * 15

    visible: false // Will be overridden in Component.onCompleted
    wideScreen: width > columnWidth * 5

    pageStack {
        initialPage: WelcomePage {
            showExisting: true
            onConnectionChosen: {
                pageStack.replace(roomListComponent);
                roomListLoaded = true;
                roomListPage = pageStack.currentItem
                RoomManager.loadInitialRoom();
            }
        }
        globalToolBar.canContainHandles: true
        defaultColumnWidth: roomListPage ? roomListPage.currentWidth : 0
        globalToolBar {
            style: Kirigami.ApplicationHeaderStyle.ToolBar
            showNavigationButtons: pageStack.currentIndex > 0 || pageStack.layers.depth > 1 ? Kirigami.ApplicationHeaderStyle.ShowBackButton : 0
        }
    }

    onConnectionChanged: {
        CustomEmojiModel.connection = root.connection
        MatrixImageProvider.connection = root.connection
        RoomManager.connection = root.connection
        SpaceHierarchyCache.connection = root.connection
    }

    Connections {
        target: LoginHelper
        function onLoaded() {
            pageStack.replace(roomListComponent);
            roomListLoaded = true;
            roomListPage = pageStack.currentItem
            RoomManager.loadInitialRoom();
        }
    }

    Connections {
        target: root.quitAction
        function onTriggered() {
            Qt.quit()
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
        onTriggered: Controller.saveWindowGeometry()
    }

    Connections {
        id: saveWindowGeometryConnections
        enabled: false // Disable on startup to avoid writing wrong values if the window is hidden
        target: root

        function onClosing() { Controller.saveWindowGeometry(); }
        function onWidthChanged() { saveWindowGeometryTimer.restart(); }
        function onHeightChanged() { saveWindowGeometryTimer.restart(); }
        function onXChanged() { saveWindowGeometryTimer.restart(); }
        function onYChanged() { saveWindowGeometryTimer.restart(); }
    }


    Loader {
        id: quickView
        active: !Kirigami.Settings.isMobile
        sourceComponent: QuickSwitcher {
            connection: root.connection
        }
    }

    Connections {
        target: RoomManager

        function onPushRoom(room, event) {
            root.roomPage = pageStack.push("qrc:/org/kde/neochat/qml/RoomPage.qml", {connection: root.connection});
            root.roomPage.forceActiveFocus();
            if (event.length > 0) {
                roomPage.goToEvent(event);
            }
        }

        function onPushSpaceHome(room) {
            root.spaceHomePage = pageStack.push("qrc:/org/kde/neochat/qml/SpaceHomePage.qml");
            root.spaceHomePage.forceActiveFocus();
        }

        function onReplaceRoom(room, event) {
            if (root.roomPage) {
                pageStack.currentIndex = pageStack.depth - 1;
            } else {
                pageStack.pop();
                root.roomPage = pageStack.push("qrc:/org/kde/neochat/qml/RoomPage.qml", {connection: root.connection});
                root.spaceHomePage = null;
            }
            root.roomPage.forceActiveFocus();
            if (event.length > 0) {
                root.roomPage.goToEvent(event);
            }
        }

        function onReplaceSpaceHome(room) {
            if (root.spaceHomePage) {
                pageStack.currentIndex = pageStack.depth - 1;
            } else {
                pageStack.pop();
                root.spaceHomePage = pageStack.push("qrc:/org/kde/neochat/qml/SpaceHomePage.qml");
                root.roomPage = null;
            }
            root.spaceHomePage.forceActiveFocus();
        }

        function goToEvent(event) {
            if (event.length > 0) {
                roomItem.goToEvent(event);
            }
            roomItem.forceActiveFocus();
        }

        function onOpenRoomInNewWindow(room) {
            const secondaryWindow = roomWindow.createObject(undefined, {currentRoom: room, connection: root.connection});
            secondaryWindow.width = root.width - pageStack.get(0).width;
            secondaryWindow.show();
        }

        function onAskDirectChatConfirmation(user) {
            askDirectChatConfirmationComponent.createObject(QQC2.ApplicationWindow.overlay, {
                user: user,
            }).open();
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
        pageStack.push("qrc:/org/kde/neochat/qml/RoomDrawerPage.qml", {
            connection: root.connection
        })
    }

    contextDrawer: RoomDrawer {
        id: contextDrawer

        // This is a memory for all user initiated actions on the drawer, i.e. clicking the button
        // It is used to ensure that user choice is remembered when changing pages and expanding and contracting the window width
        property bool drawerUserState: Config.autoRoomInfoDrawer

        connection: root.connection

        // Connect to the onClicked function of the RoomDrawer handle button
        Connections {
            target: contextDrawer.handle.children[0]
            function onClicked() {
                contextDrawer.drawerUserState = contextDrawer.drawerOpen
            }
        }

        modal: (!root.wideScreen || !enabled)
        onEnabledChanged: drawerOpen = enabled && !modal
        onModalChanged: {
            if (Config.autoRoomInfoDrawer) {
                drawerOpen = !modal && drawerUserState
                dim = false
            }
        }
        enabled: RoomManager.hasOpenRoom && pageStack.layers.depth < 2 && pageStack.depth < 3 && (pageStack.visibleItems.length > 1 || pageStack.currentIndex > 0) && !Kirigami.Settings.isMobile && root.pageStack.wideMode
        handleVisible: enabled
    }

    Component.onCompleted: {
        CustomEmojiModel.connection = root.connection
        MatrixImageProvider.connection = root.connection
        RoomManager.connection = root.connection
        SpaceHierarchyCache.connection = root.connection
        WindowController.setBlur(pageStack, Config.blur && !Config.compactLayout);
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
                RoomManager.reset();
                pageStack.clear();
                roomListLoaded = false;
                pageStack.push("qrc:/org/kde/neochat/qml/WelcomePage.qml");
            }
        }
    }

    Connections {
        target: Controller

        function onErrorOccured(error, detail) {
            showPassiveNotification(detail.length > 0 ? i18n("%1: %2", error, detail) : error);
        }

        function onUserConsentRequired(url) {
            let consent = consentSheetComponent.createObject(QQC2.ApplicationWindow.overlay)
            consent.url = url
            consent.open()
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

    Component {
        id: keyVerificationDialogComponent
        KeyVerificationDialog { }
    }

    Connections {
        target: root.connection

        function onDirectChatAvailable(directChat) {
            RoomManager.enterRoom(root.connection.room(directChat.id));
        }
        function onNewKeyVerificationSession(session) {
            applicationWindow().pageStack.pushDialogLayer(keyVerificationDialogComponent, {
                session: session,
            }, {
                title: i18nc("@title:window", "Session Verification")
            });
        }
    }

    Component {
        id: consentSheetComponent
        Kirigami.OverlaySheet {
            id: consentSheet

            property string url: ""

            title: i18n("User consent")

            QQC2.Label {
                id: label

                text: i18n("Your homeserver requires you to agree to its terms and conditions before being able to use it. Please click the button below to read them.")
                wrapMode: Text.WordWrap
                width: parent.width
            }
            footer: QQC2.Button {
                text: i18n("Open")
                onClicked: UrlHelper.openUrl(consentSheet.url)
            }
        }
    }

    Component {
        id: createRoomDialog

        CreateRoomDialog {
            connection: root.connection
        }
    }

    Component {
        id: roomWindow
        RoomWindow {}
    }

    Component {
        id: askDirectChatConfirmationComponent

        Kirigami.OverlaySheet {
            id: askDirectChatConfirmation
            required property var user;

            parent: QQC2.ApplicationWindow.overlay
            title: i18n("Start a chat")
            contentItem: QQC2.Label {
                text: i18n("Do you want to start a chat with %1?", user.displayName)
                wrapMode: Text.WordWrap
            }
            footer: QQC2.DialogButtonBox {
                standardButtons: QQC2.DialogButtonBox.Ok | QQC2.DialogButtonBox.Cancel
                onAccepted: {
                    user.requestDirectChat();
                    askDirectChatConfirmation.close();
                }
                onRejected: askDirectChatConfirmation.close();
            }
        }
    }

    property Item hoverLinkIndicator: QQC2.Control {
        parent: overlay.parent 
        property string text
        opacity: linkText.text.length > 0 ? 1 : 0

        z: 20
        x: 0
        y: parent.height - implicitHeight
        contentItem: QQC2.Label {
            id: linkText
            text: parent.text.startsWith("https://matrix.to/") ? "" : parent.text
        }
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        background: Rectangle {
             color: Kirigami.Theme.backgroundColor
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+,"
        onActivated: {
            pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/SettingsPage.qml", {connection: root.connection}, { title: i18n("Configure") })
        }
    }
}
