// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0
import NeoChat.Dialog 1.0
import NeoChat.Page 1.0
import NeoChat.Panel 1.0

Kirigami.ApplicationWindow {
    id: root

    property int columnWidth: Kirigami.Units.gridUnit * 13

    minimumWidth: Kirigami.Units.gridUnit * 15
    minimumHeight: Kirigami.Units.gridUnit * 20

    wideScreen: width > columnWidth * 5

    onClosing: Controller.saveWindowGeometry(root)

    pageStack.initialPage: LoadingPage {}
    pageStack.globalToolBar.canContainHandles: true

    property bool roomListLoaded: false

    Connections {
        target: root.quitAction
        function onTriggered() {
            Qt.quit()
        }
    }

    Loader {
        active: !Kirigami.Settings.isMobile
        source: Qt.resolvedUrl("qrc:/imports/NeoChat/Menu/GlobalMenu.qml")
    }

    // This timer allows to batch update the window size change to reduce
    // the io load and also work around the fact that x/y/width/height are
    // changed when loading the page and overwrite the saved geometry from
    // the previous session.
    Timer {
        id: saveWindowGeometryTimer
        interval: 1000
        onTriggered: Controller.saveWindowGeometry(root)
    }

    onWidthChanged: saveWindowGeometryTimer.restart()
    onHeightChanged: saveWindowGeometryTimer.restart()
    onXChanged: saveWindowGeometryTimer.restart()
    onYChanged: saveWindowGeometryTimer.restart()

    Shortcut {
        sequence: "Ctrl+K"
        onActivated: {
            quickView.item.open()
        }
    }

    Loader {
        id: quickView

        active: !Kirigami.Settings.isMobile
        sourceComponent: QuickSwitcher { }
    }

    /// Setup keyboard navigation to the room page.
    function connectRoomToSignal(item) {
        if (!roomListLoaded) {
            console.log("Should not happen: no room list page but room page");
        }
        const roomList = pageStack.get(0);
        item.switchRoomUp.connect(function() {
            roomList.goToNextRoom();
        });

        item.switchRoomDown.connect(function() {
            roomList.goToPreviousRoom();
        });
        item.forceActiveFocus();
        item.KeyNavigation.left = pageStack.get(0);
    }

    Connections {
        target: RoomManager

        function onPushRoom(room, event) {
            const roomItem = pageStack.push("qrc:/imports/NeoChat/Page/RoomPage.qml");
            connectRoomToSignal(roomItem);
            if (event.length > 0) {
                roomItem.goToEvent(event);
            }
        }

        function onReplaceRoom(room, event) {
            const roomItem = pageStack.get(pageStack.depth - 1);
            pageStack.currentIndex = pageStack.depth - 1;
            connectRoomToSignal(roomItem);
            if (event.length > 0) {
                roomItem.goToEvent(event);
            }
        }

        function goToEvent(event) {
            if (event.length > 0) {
                roomItem.goToEvent(event);
            }
            roomItem.forceActiveFocus();
        }

        function onPushWelcomePage() {
            // TODO
        }

        function onOpenRoomInNewWindow(room) {
            const secondayWindow = roomWindow.createObject(applicationWindow(), {currentRoom: room});
            secondayWindow.width = root.width - pageStack.get(0).width;
            secondayWindow.show();
        }

        function onShowUserDetail(user) {
            const roomItem = pageStack.get(pageStack.depth - 1);
            roomItem.showUserDetail(user);
        }

        function onAskDirectChatConfirmation(user) {
            askDirectChatConfirmationComponent.createObject(QQC2.ApplicationWindow.overlay, {
                user: user,
            }).open();
        }

        function onWarning(title, message) {
            if (RoomManager.currentRoom) {
                const roomItem = pageStack.get(pageStack.depth - 1);
                roomItem.warning(title, message);
            } else {
                showPassiveNotification(i18n("Warning: %1", message));
            }
        }
    }

    function pushReplaceLayer(page, args) {
        if (pageStack.layers.depth === 2) {
            pageStack.layers.replace(page, args);
        } else {
            pageStack.layers.push(page, args);
        }
    }

    function showWindow() {
        root.show()
        root.raise()
        root.requestActivate()
        Controller.raiseWindow(root)
    }

    contextDrawer: RoomDrawer {
        id: contextDrawer
        contentItem.implicitWidth: columnWidth
        edge: Qt.application.layoutDirection == Qt.RightToLeft ? Qt.LeftEdge : Qt.RightEdge
        modal: !root.wideScreen || !enabled
        onEnabledChanged: drawerOpen = enabled && !modal
        onModalChanged: drawerOpen = !modal
        enabled: RoomManager.hasOpenRoom && pageStack.layers.depth < 2 && pageStack.depth < 3
        handleVisible: enabled && pageStack.layers.depth < 2 && pageStack.depth < 3
    }

    readonly property int defaultPageWidth: Kirigami.Units.gridUnit * 17
    readonly property int minPageWidth: Kirigami.Units.gridUnit * 10
    readonly property int collapsedPageWidth: Kirigami.Units.gridUnit * 3 - Kirigami.Units.smallSpacing * 3
    readonly property bool shouldUseSidebars: RoomManager.hasOpenRoom && (Config.roomListPageWidth > minPageWidth ? root.width >= Kirigami.Units.gridUnit * 35 : root.width > Kirigami.Units.gridUnit * 27) && roomListLoaded
    readonly property int pageWidth: {
        if (Config.roomListPageWidth === -1) {
            return defaultPageWidth;
        } else if (Config.roomListPageWidth < minPageWidth) {
            return collapsedPageWidth;
        } else {
            return Config.roomListPageWidth;
        }
    }

    pageStack.defaultColumnWidth: pageWidth
    pageStack.columnView.columnResizeMode: shouldUseSidebars ? Kirigami.ColumnView.FixedColumns : Kirigami.ColumnView.SingleColumn

    MouseArea {
        visible: root.pageStack.wideMode
        z: 500

        anchors.top: parent.top
        anchors.bottom: parent.bottom

        x: root.pageStack.defaultColumnWidth - (width / 2)
        width: 2

        property int _lastX: -1
        enabled: !Kirigami.Settings.isMobile

        cursorShape: !Kirigami.Settings.isMobile ? Qt.SplitHCursor : undefined

        onPressed: _lastX = mouseX
        onReleased: Config.save();

        onPositionChanged: {
            if (_lastX == -1) {
                return;
            }

            if (mouse.x > _lastX) {
                // we moved to the right
                if (Config.roomListPageWidth === root.collapsedPageWidth && root.pageWidth + (mouse.x - _lastX) >= root.minPageWidth) {
                    // Here we get back directly to a more wide mode.
                    Config.roomListPageWidth = root.minPageWidth;
                    if (root.width < Kirigami.Units.gridUnit * 35) {
                        root.width = Kirigami.Units.gridUnit * 35;
                    }
                } else if (Config.roomListPageWidth !== root.collapsedPageWidth) {
                    // Increase page width
                    Config.roomListPageWidth = Math.min(root.defaultPageWidth, root.pageWidth + (mouse.x - _lastX));
                }
            } else if (mouse.x < _lastX) {
                const tmpWidth = root.pageWidth - (_lastX - mouse.x);

                if (tmpWidth < root.minPageWidth) {
                    Config.roomListPageWidth = root.collapsedPageWidth;
                } else {
                    Config.roomListPageWidth = tmpWidth;
                }
            }
        }
    }


    globalDrawer: Kirigami.GlobalDrawer {
        property bool hasLayer
        contentItem.implicitWidth: columnWidth
        isMenu: true
        actions: [
            Kirigami.Action {
                text: i18n("Explore rooms")
                icon.name: "compass"
                onTriggered: pushReplaceLayer("qrc:/imports/NeoChat/Page/JoinRoomPage.qml", {"connection": Controller.activeConnection})
                enabled: pageStack.layers.currentItem.title !== i18n("Explore Rooms") && Controller.accountCount > 0
            },
            Kirigami.Action {
                text: i18n("Start a Chat")
                icon.name: "irc-join-channel"
                onTriggered: pushReplaceLayer("qrc:/imports/NeoChat/Page/StartChatPage.qml", {"connection": Controller.activeConnection})
                enabled: pageStack.layers.currentItem.title !== i18n("Start a Chat") && Controller.accountCount > 0
            },
            Kirigami.Action {
                text: i18n("Create a Room")
                icon.name: "irc-join-channel"
                onTriggered: {
                    let dialog = createRoomDialog.createObject(root.overlay);
                    dialog.open();
                }
                shortcut: StandardKey.New
                enabled: pageStack.layers.currentItem.title !== i18n("Start a Chat") && Controller.accountCount > 0
            },
            Kirigami.Action {
                text: i18n("Settings")
                icon.name: "settings-configure"
                onTriggered: pushReplaceLayer("qrc:/imports/NeoChat/Page/SettingsPage.qml")
                enabled: pageStack.layers.currentItem.title !== i18n("Settings")
                shortcut: StandardKey.Preferences
            },
            Kirigami.Action {
                text: i18n("Logout")
                icon.name: "list-remove-user"
                enabled: Controller.accountCount > 0
                onTriggered: Controller.logout(Controller.activeConnection, true)
            },
            Kirigami.Action {
                text: i18n("Quit")
                icon.name: "gtk-quit"
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }
        ]
    }

    Component.onCompleted: Controller.setBlur(pageStack, Config.blur && !Config.compactLayout);
    Connections {
        target: Config
        function onBlurChanged() {
            Controller.setBlur(pageStack, Config.blur && !Config.compactLayout);
        }
        function onCompactLayoutChanged() {
            Controller.setBlur(pageStack, Config.blur && !Config.compactLayout);
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
        }
    }

    Connections {
        target: Controller

        function onInitiated() {
            if (Controller.accountCount === 0) {
                pageStack.replace("qrc:/imports/NeoChat/Page/WelcomePage.qml", {});
            } else {
                pageStack.replace(roomListComponent, {
                    activeConnection: Controller.activeConnection
                });
                roomListLoaded = true;
                RoomManager.loadInitialRoom();
            }
        }

        function onConnectionDropped() {
            if (Controller.accountCount === 0) {
                RoomManager.reset();
                pageStack.clear();
                roomListLoaded = false;
                pageStack.replace("qrc:/imports/NeoChat/Page/WelcomePage.qml");
            }
        }

        function onGlobalErrorOccured(error, detail) {
            showPassiveNotification(i18nc("%1: %2", error, detail));
        }

        function onShowWindow() {
            root.showWindow()
            root.raise()
        }

        function onUserConsentRequired(url) {
            consentSheet.url = url
            consentSheet.open()
        }
    }

    Connections {
        target: Controller.activeConnection
        function onDirectChatAvailable(directChat) {
            RoomManager.enterRoom(Controller.activeConnection.room(directChat.id));
        }
    }

    Kirigami.OverlaySheet {
        id: consentSheet

        property string url: ""

        header: Kirigami.Heading {
            text: i18n("User consent")
        }

        QQC2.Label {
            id: label

            text: i18n("Your homeserver requires you to agree to its terms and conditions before being able to use it. Please click the button below to read them.")
            wrapMode: Text.WordWrap
            width: parent.width
        }
        footer: QQC2.Button {
            text: i18n("Open")
            onClicked: Qt.openUrlExternally(consentSheet.url)
        }
    }

    Component {
        id: createRoomDialog

        CreateRoomDialog {}
    }

    Component {
        id: roomWindow
        RoomWindow {}
    }

    Component {
        id: userDialog
        UserDetailDialog {}
    }

    Component {
        id: askDirectChatConfirmationComponent

        Kirigami.OverlaySheet {
            id: askDirectChatConfirmation
            required property var user;

            parent: QQC2.ApplicationWindow.overlay
            header: Kirigami.Heading {
                text: i18n("Start a chat")
            }
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
        property alias text: linkText.text
        opacity: text.length > 0 ? 1 : 0

        z: 20
        x: 0
        y: parent.height - implicitHeight
        contentItem: QQC2.Label {
            id: linkText
        }
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        background: Rectangle {
             color: Kirigami.Theme.backgroundColor
        }
    }
}
