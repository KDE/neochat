// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat.libneochat
import org.kde.neochat

/**
 * @brief Component for visualising the room information.
 *
 * The component has a header section which changes between group rooms and direct
 * chats with information like the avatar and topic. Followed by the allowed actions
 * and finally a user list.
 *
 * @note This component is only the contents, it will need to be placed in either
 *       a drawer (desktop) or page (mobile) to be used.
 *
 * @sa RoomDrawer, RoomDrawerPage
 */
QQC2.ScrollView {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom room

    required property UserListModel userListModel

    /**
     * @brief The title that should be displayed for this component if available.
     */
    readonly property string title: root.room.isSpace ? i18nc("@action:title", "Space Members") : i18nc("@action:title", "Room Information")

    signal resolveResource(string idOrUri, string action)

    // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
    QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

    ListView {
        id: userList

        // Used to determine if the view has settled and should stop perfoming the hack
        property bool viewHasSettled: false

        header: ColumnLayout {
            id: columnLayout

            property alias userListSearchField: userListSearchField

            spacing: 0
            width: ListView.view ? ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin : 0

            // HACK: Resettle this ListView while our labels wrap themselves. ListView doesn't do this automatically.
            // We use the Timer to determine when its done internally reshuffling, so we don't accidentally send you to the top if you actually scrolled down.
            onHeightChanged: {
                if (!userList.viewHasSettled) {
                    userList.positionViewAtBeginning();
                    hackTimer.restart();
                }
            }

            Timer {
                id: hackTimer

                // The internal wrapping and height changes happen quickly, so we don't need a long interval here
                interval: 1
                onTriggered: userList.viewHasSettled = true
            }

            Loader {
                active: true
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.smallSpacing
                visible: !root.room.isSpace
                sourceComponent: root.room.isDirectChat() ? directChatDrawerHeader : groupChatDrawerHeader
            }

            Kirigami.ListSectionHeader {
                visible: !root.room.isSpace
                text: i18nc("Room actions", "Actions")
                activeFocusOnTab: false

                Layout.fillWidth: true
            }

            Delegates.RoundedItemDelegate {
                id: searchButton
                visible: !root.room.isSpace
                icon.name: "search"
                text: i18nc("@action:button", "Search Messages")
                activeFocusOnTab: true

                Layout.fillWidth: true

                onClicked: {
                    ((QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'RoomSearchPage'), {
                        room: root.room
                    }, {
                        title: i18nc("@action:title", "Search")
                    });
                }
            }

            Delegates.RoundedItemDelegate {
                visible: root.room.isDirectChat()
                icon.name: "security-low-symbolic"
                text: i18nc("@action:button", "Verify user")

                onClicked: root.room.startVerification()

                Layout.fillWidth: true
            }

            Delegates.RoundedItemDelegate {
                id: widgetsButton
                visible: !root.room.isSpace
                icon.name: "extension-symbolic"
                text: i18nc("@action:button", "Extensions")
                activeFocusOnTab: true

                onClicked: ((QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'WidgetsPage'), {
                    room: root.room
                }, {
                    title: i18nc("@title:window", "Extensions")
                })

                Layout.fillWidth: true
            }

            Delegates.RoundedItemDelegate {
                id: locationsButton
                visible: !root.room.isSpace
                icon.name: "mark-location-symbolic"
                text: i18nc("@action:button", "Shared Locations")
                activeFocusOnTab: true

                onClicked: ((QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'LocationsPage'), {
                    room: root.room
                }, {
                    title: i18nc("Locations on a map", "Locations")
                })

                Layout.fillWidth: true
            }

            Delegates.RoundedItemDelegate {
                id: pinnedMessagesButton
                // On mobile the pinned message at the top is hidden, so we need this button in that case.
                visible: !root.room.isSpace && Kirigami.Settings.isMobile
                icon.name: "pin-symbolic"
                text: i18nc("@action:button", "Pinned Messages")
                activeFocusOnTab: true

                Layout.fillWidth: true

                onClicked: {
                    ((QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'RoomPinnedMessagesPage'), {
                        room: root.room
                    }, {
                        title: i18nc("@title", "Pinned Messages")
                    });
                }
            }

            Delegates.RoundedItemDelegate {
                text: i18nc("@action:inmenu", "Inspect Room Data")
                icon.name: "tools"
                visible: NeoChatConfig.developerTools
                activeFocusOnTab: true

                Layout.fillWidth: true

                onClicked: ((QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat.devtools', 'DevtoolsPage'), {
                    connection: root.room.connection,
                    currentTabIndex: 1, // Room data tab
                    room: root.room
                }, {
                    title: i18nc("@title:window", "Developer Tools"),
                    width: Kirigami.Units.gridUnit * 50,
                    height: Kirigami.Units.gridUnit * 42
                })
            }

            Delegates.RoundedItemDelegate {
                id: leaveButton
                icon.name: "arrow-left-symbolic"
                text: root.room.isSpace ? i18nc("@action:button", "Leave Space…") : i18nc("@action:button", "Leave Room…")
                activeFocusOnTab: true

                Layout.fillWidth: true

                onClicked: {
                    (Qt.createComponent('org.kde.neochat', 'ConfirmLeaveDialog').createObject(root.QQC2.ApplicationWindow.window, {
                        room: root.room
                    }) as ConfirmLeaveDialog).open();
                }
            }

            Kirigami.ListSectionHeader {
                text: i18n("Members")
                activeFocusOnTab: false
                spacing: 0
                visible: !root.room.isDirectChat()

                Layout.fillWidth: true

                QQC2.ToolButton {
                    visible: root.room.canSendState("invite")
                    icon.name: "list-add-user"

                    onClicked: {
                        ((QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'InviteUserPage'), {
                            room: root.room
                        }, {
                            title: i18nc("@title", "Invite a User")
                        });
                    }

                    QQC2.ToolTip.text: i18n("Invite user to room")
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                QQC2.Label {
                    Layout.alignment: Qt.AlignRight
                    text: root.room ? i18ncp("@info", "%1 member", "%1 members", root.room.joinedCount) : i18n("No member count")
                }
            }

            Kirigami.SearchField {
                id: userListSearchField

                visible: !root.room.isDirectChat()
                onVisibleChanged: if (visible) {
                    forceActiveFocus();
                }
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing

                focusSequence: "Ctrl+Shift+F"

                onAccepted: userFilterModel.filterText = text
            }
        }

        model: root.room.isDirectChat() ? 0 : userFilterModel

        UserFilterModel {
            id: userFilterModel
            sourceModel: root.userListModel
            allowEmpty: true
        }

        clip: true
        focus: true

        section.property: "powerLevelString"
        section.delegate: Kirigami.ListSectionHeader {
            required property string section

            width: ListView.view.width
            text: section
        }

        delegate: Delegates.RoundedItemDelegate {
            id: userDelegate

            required property int index
            required property string name
            required property string userId
            required property url avatar
            required property int powerLevel
            required property string powerLevelString
            required property color color

            implicitHeight: Kirigami.Units.gridUnit * 2

            text: name

            KeyNavigation.tab: navigationBar.tabGroup.checkedButton
            KeyNavigation.backtab: index === 0 ? userList.headerItem.userListSearchField : null

            onClicked: {
                root.resolveResource(userDelegate.userId, "mention");
            }

            contentItem: RowLayout {
                KirigamiComponents.Avatar {
                    implicitWidth: height
                    sourceSize {
                        height: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                        width: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                    }
                    source: userDelegate.avatar
                    name: userDelegate.userId
                    color: userDelegate.color

                    Layout.fillHeight: true
                }
                QQC2.Label {
                    text: userDelegate.name
                    textFormat: Text.PlainText
                    elide: Text.ElideRight
                    clip: true // Intentional to limit insane Unicode in display names

                    Layout.fillWidth: true
                }
            }
        }
    }

    Component {
        id: groupChatDrawerHeader
        GroupChatDrawerHeader {
            room: root.room
        }
    }

    Component {
        id: directChatDrawerHeader
        DirectChatDrawerHeader {
            room: root.room

            onResolveResource: (idOrUri, action) => root.resolveResource(idOrUri, action)
        }
    }

    onRoomChanged: {
        if (userList.headerItem) {
            userList.headerItem.userListSearchField.text = "";
        }
        userList.currentIndex = -1;
        userList.viewHasSettled = false;
    }
}
