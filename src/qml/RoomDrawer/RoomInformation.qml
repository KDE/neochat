// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.delegates 1.0 as Delegates
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

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

    /**
     * @brief The title that should be displayed for this component if available.
     */
    readonly property string title: i18nc("@action:title", "Room information")

    // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
    QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

    ListView {
        id: userList
        header: ColumnLayout {
            id: columnLayout

            property alias userListSearchField: userListSearchField

            spacing: 0
            width: ListView.view ? ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin : 0

            Loader {
                active: true
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.smallSpacing
                sourceComponent: root.room.isDirectChat() ? directChatDrawerHeader : groupChatDrawerHeader
                onItemChanged: if (item) {
                    userList.positionViewAtBeginning();
                }
            }

            Kirigami.ListSectionHeader {
                label: i18n("Options")
                activeFocusOnTab: false

                Layout.fillWidth: true
            }

            Delegates.RoundedItemDelegate {
                id: devtoolsButton

                icon.name: "tools"
                text: i18n("Open developer tools")
                visible: Config.developerTools

                Layout.fillWidth: true

                onClicked: {
                    applicationWindow().pageStack.pushDialogLayer("qrc:/DevtoolsPage.qml", {room: root.room}, {title: i18n("Developer Tools")})
                }
            }

            Delegates.RoundedItemDelegate {
                id: searchButton

                icon.name: "search"
                text: i18n("Search in this room")

                Layout.fillWidth: true

                onClicked: {
                    pageStack.pushDialogLayer("qrc:/SearchPage.qml", {
                        currentRoom: root.room
                    }, {
                        title: i18nc("@action:title", "Search")
                    })
                }
            }

            Delegates.RoundedItemDelegate {
                id: favouriteButton

                icon.name: root.room && root.room.isFavourite ? "rating" : "rating-unrated"
                text: root.room && root.room.isFavourite ? i18n("Remove room from favorites") : i18n("Make room favorite")

                onClicked: root.room.isFavourite ? root.room.removeTag("m.favourite") : root.room.addTag("m.favourite", 1.0)

                Layout.fillWidth: true
            }

            Delegates.RoundedItemDelegate {
                id: locationsButton

                icon.name: "map-flat"
                text: i18n("Show locations for this room")

                onClicked: pageStack.pushDialogLayer("qrc:/LocationsPage.qml", {
                    room: root.room
                }, {
                    title: i18nc("Locations on a map", "Locations")
                })

                Layout.fillWidth: true
            }

            Kirigami.ListSectionHeader {
                label: i18n("Members")
                activeFocusOnTab: false
                spacing: 0
                visible: !root.room.isDirectChat()

                Layout.fillWidth: true

                QQC2.ToolButton {
                    visible: root.room.canSendState("invite")
                    icon.name: "list-add-user"

                    onClicked: {
                        applicationWindow().pageStack.pushDialogLayer("qrc:/InviteUserPage.qml", {room: root.room}, {title: i18nc("@title", "Invite a User")})
                    }

                    QQC2.ToolTip.text: i18n("Invite user to room")
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                QQC2.Label {
                    Layout.alignment: Qt.AlignRight
                    text: root.room ? i18np("%1 member", "%1 members", root.room.joinedCount) : i18n("No member count")
                }
            }

            Kirigami.SearchField {
                id: userListSearchField

                onVisibleChanged: if (visible) forceActiveFocus()
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing

                focusSequence: "Ctrl+Shift+F"

                onAccepted: sortedMessageEventModel.filterString = text;
            }
        }

        KSortFilterProxyModel {
            id: sortedMessageEventModel

            sourceModel: UserListModel {
                room: root.room
            }

            sortRole: "powerLevel"
            sortOrder: Qt.DescendingOrder
            filterRole: "name"
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        model: root.room.isDirectChat() ? 0 : sortedMessageEventModel

        clip: true
        activeFocusOnTab: true

        delegate: Delegates.RoundedItemDelegate {
            id: userDelegate

            required property string name
            required property string userId
            required property string avatar
            required property int powerLevel
            required property string powerLevelString

            implicitHeight: Kirigami.Units.gridUnit * 2

            text: name

            onClicked: {
                userDelegate.highlighted = true;
                RoomManager.visitUser(room.getUser(userDelegate.userId).object, "mention")
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

                    Layout.fillHeight: true
                }

                QQC2.Label {
                    text: userDelegate.name
                    textFormat: Text.PlainText
                    elide: Text.ElideRight

                    Layout.fillWidth: true
                }

                QQC2.Label {
                    visible: userDelegate.powerLevel > 0

                    text: userDelegate.powerLevelString
                    color: Kirigami.Theme.disabledTextColor
                    textFormat: Text.PlainText
                }
            }
        }
    }

    Component {
        id: groupChatDrawerHeader
        GroupChatDrawerHeader {}
    }

    Component {
        id: directChatDrawerHeader
        DirectChatDrawerHeader {}
    }

    onRoomChanged: {
        if (root.headerItem) {
            root.headerItem.userListSearchField.text = "";
        }
        userList.currentIndex = -1
    }
}
