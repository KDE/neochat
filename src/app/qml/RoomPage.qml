// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Window
import QtQuick.Layouts

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

    /**
     * @brief The WidgetModel to use.
     *
     * This model has the list of widgets available in the current room.
     *
     * @note For loading a room in a different window, override this with a new
     *       WidgetModel.
     *
     * @sa WidgetModel
     */
    property WidgetModel widgetModel: RoomManager.widgetModel

    title: root.currentRoom ? root.currentRoom.displayName : ""
    focus: true
    padding: 0

    actions: [
        Kirigami.Action {
            tooltip: i18nc("@action:button", "Open Jitsi Meet in browser")
            icon.name: "camera-video-symbolic"
            onTriggered: {
                let url
                if (root.widgetModel.jitsiIndex < 0) {
                    url = root.widgetModel.addJitsiConference();
                } else {
                    let idx = root.widgetModel.index(root.widgetModel.jitsiIndex, 0);
                    url = root.widgetModel.data(idx, WidgetModel.UrlRole);
                }
                Qt.openUrlExternally(url);
            }
        },
        Kirigami.Action {
            visible: Kirigami.Settings.isMobile || !(root.Kirigami.PageStack.pageStack as Kirigami.PageRow).wideMode
            icon.name: "view-right-new"
            onTriggered: (root.QQC2.ApplicationWindow.window as Main).openRoomDrawer()
        }
    ]

    KeyNavigation.left: (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).get(0)

    onCurrentRoomChanged: {
        if (!Kirigami.Settings.isMobile && chatBarLoader.item) {
            (chatBarLoader.item as ChatBar).forceActiveFocus();
        }

        if (root.currentRoom.tagNames.includes("m.server_notice")) {
            banner.text = i18nc("@info", "This room contains official messages from your homeserver.")
            banner.show("message");
        } else {
            banner.hideIf("message");
        }
    }

    Connections {
        target: root.currentRoom.connection
        function onIsOnlineChanged() {
            if (!root.currentRoom.connection.isOnline) {
                banner.text = i18nc("@info:status", "NeoChat is offline. Please check your network connection.");
                banner.type = Kirigami.MessageType.Error;
                banner.show("offline");
            } else {
                banner.hideIf("offline");
            }
        }
    }

    header: ColumnLayout {
        id: headerLayout

        spacing: 0

        readonly property bool shouldShowPins: root.currentRoom.pinnedMessage.length > 0 && !Kirigami.Settings.isMobile

        QQC2.Control {
            id: pinControl

            visible: headerLayout.shouldShowPins

            Layout.fillWidth: true

            background: Rectangle {
                color: Kirigami.Theme.backgroundColor

                Kirigami.Theme.colorSet: Kirigami.Theme.View
                Kirigami.Theme.inherit: false
            }

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Icon {
                    source: "pin-symbolic"

                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                }

                QQC2.Label {
                    text: root.currentRoom.pinnedMessage
                    maximumLineCount: 1
                    elide: Text.ElideRight

                    onLinkActivated: link => UrlHelper.openUrl(link)
                    onHoveredLinkChanged: if (hoveredLink.length > 0 && hoveredLink !== "1") {
                        (QQC2.ApplicationWindow.window as Main).hoverLinkIndicator.text = hoveredLink;
                    } else {
                        (QQC2.ApplicationWindow.window as Main).hoverLinkIndicator.text = "";
                    }

                    Layout.fillWidth: true
                }
            }

            TapHandler {
                onTapped: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'RoomPinnedMessagesPage'), {
                    room: root.currentRoom
                }, {
                    title: i18nc("@title", "Pinned Messages")
                });
            }
        }

        Kirigami.Separator {
            visible: headerLayout.shouldShowPins

            Layout.fillWidth: true
        }

        Kirigami.InlineMessage {
            id: banner

            // Used to keep track of messages so we can hide the right one at the right time
            property string messageId

            showCloseButton: true
            visible: false
            position: Kirigami.InlineMessage.Position.Header

            function show(msgid: string): void {
                messageId = msgid;
                visible = true;
            }

            function hideIf(msgid: string): void {
                if (messageId == msgid) {
                    visible = false;
                }
            }
        }
    }

    Loader {
        id: timelineViewLoader
        anchors.fill: parent
        // We need the loader to be active but invisible while the room is loading messages so signals in TimelineView work.
        active: root.currentRoom && !root.currentRoom.isInvite && !root.currentRoom.isSpace
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

    footer: Loader {
        id: chatBarLoader
        height: active ? (item as ChatBar).implicitHeight : 0
        active: timelineViewLoader.active && !root.currentRoom.readOnly
        sourceComponent: ChatBar {
            id: chatBar
            width: parent.width
            currentRoom: root.currentRoom
            connection: root.currentRoom.connection as NeoChatConnection
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
            (timelineViewLoader.item as TimelineView).pageUp();
        } else if (event.key === Qt.Key_PageDown) {
            event.accepted = true;
            (timelineViewLoader.item as TimelineView).pageDown();
        }
    }

    Connections {
        target: RoomManager

        function onShowMessage(messageType, message) {
            banner.text = message;
            banner.type = messageType;
            banner.show("generic");
        }

        function onShowEventSource(eventId) {
            (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'MessageSourceSheet'), {
                sourceText: root.currentRoom.getEventJsonSource(eventId)
            }, {
                title: i18nc("@title:dialog", "Message Source"),
                width: Kirigami.Units.gridUnit * 25
            });
        }

        function onShowDelegateMenu(eventId: string, author, messageComponentType, plainText: string, richText: string, mimeType: string, progressInfo, isThread: bool, selectedText: string, hoveredLink: string) {
            (delegateContextMenu.createObject(root, {
                author: author,
                eventId: eventId,
                plainText: plainText,
                mimeType: mimeType,
                progressInfo: progressInfo,
                messageComponentType: messageComponentType,
            }) as DelegateContextMenu).popup();
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
            (Qt.createComponent('org.kde.neochat', 'CodeMaximizeComponent').createObject(QQC2.Overlay.overlay, {
                author: author,
                time: time,
                codeText: codeText,
                language: language
            }) as CodeMaximizeComponent).open();
        }
    }

    Component {
        id: delegateContextMenu
        DelegateContextMenu {
            room: root.currentRoom
            connection: root.currentRoom.connection as NeoChatConnection
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
