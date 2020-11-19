/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.13 as Kirigami
import org.kde.kitemmodels 1.0
import org.kde.neochat 1.0

import NeoChat.Component 1.0
import NeoChat.Menu 1.0

Kirigami.ScrollablePage {
    id: page

    property var roomListModel
    property var enteredRoom
    required property var activeConnection

    signal enterRoom(var room)
    signal leaveRoom(var room)

    title: i18n("Rooms")

    titleDelegate: Kirigami.SearchField {
        Layout.topMargin: Kirigami.Units.smallSpacing
        Layout.bottomMargin: Kirigami.Units.smallSpacing
        Layout.fillHeight: true
        Layout.fillWidth: true
        onTextChanged: sortFilterRoomListModel.filterText = text
        KeyNavigation.tab: listView
    }

    ListView {
        id: listView
        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: listView.count == 0
            text: sortFilterRoomListModel.filterText.length > 0 ? i18n("No room found.") : i18n("You didn't join any room yet.")
            helpfulAction: Kirigami.Action {
                icon.name: "list-add"
                text: i18n("Explore rooms")
                onTriggered: pageStack.layers.push("qrc:/imports/NeoChat/Page/JoinRoomPage.qml", {"connection": activeConnection})
            }
        }
        model:  SortFilterRoomListModel {
            id: sortFilterRoomListModel
            sourceModel: roomListModel
            roomSortOrder: SortFilterRoomListModel.Categories
        }

        section.property: "category"
        section.delegate: Kirigami.ListSectionHeader {
            id: sectionHeader
            action: Kirigami.Action {
                onTriggered: roomListModel.setCategoryVisible(section, !roomListModel.categoryVisible(section))
            }
            contentItem: Item {
                implicitHeight: categoryName.implicitHeight
                Kirigami.Heading {
                    id: categoryName
                    level: 3
                    text: roomListModel.categoryName(section)
                }
                Kirigami.Icon {
                    source: roomListModel.categoryVisible(section) ? "go-up" : "go-down"
                    implicitHeight: Kirigami.Units.iconSizes.small
                    implicitWidth: Kirigami.Units.iconSizes.small
                    anchors.left: categoryName.right
                    anchors.leftMargin: Kirigami.Units.largeSpacing
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        delegate: Kirigami.SwipeListItem {
            visible: model.categoryVisible
            height: model.categoryVisible ? implicitHeight : 0
            focus: true
            action: Kirigami.Action {
                id: enterRoomAction
                onTriggered: {
                    if (category === RoomType.Invited) {
                        roomManager.openInvitation(currentRoom);
                    } else {
                        var roomItem = roomManager.enterRoom(currentRoom)
                        roomListItem.KeyNavigation.right = roomItem
                        roomItem.focus = true;
                    }
                }
            }
            actions: [
                Kirigami.Action {
                    id: makeAction
                    text: currentRoom.isFavourite ? i18n("Remove room from favorites") : i18n("Make room favorite")

                    icon.name: currentRoom.isFavourite ? "rating" : "rating-unrated"
                    checkable: true
                    checked: currentRoom.isFavourite

                    onTriggered: currentRoom.isFavourite ? currentRoom.removeTag("m.favourite") : currentRoom.addTag("m.favourite", 1.0)

                },
                Kirigami.Action {
                    id: optionAction

                    text: i18n("Configure room")

                    icon.name: "configure"
                    onTriggered: roomListContextMenu.createObject(roomLayout, {"room": currentRoom}).popup();
                }
            ]
            contentItem: Item {
                id: listItem
                focus: true
                implicitHeight: roomLayout.implicitHeight
                RowLayout {
                    id: roomLayout
                    anchors.fill: parent
                    spacing: Kirigami.Units.largeSpacing

                    Kirigami.Avatar {
                        id: roomAvatar
                        property int size: Kirigami.Units.gridUnit * 3 - Kirigami.Units.smallSpacing * 2;
                        Layout.minimumHeight: size
                        Layout.maximumHeight: size
                        Layout.minimumWidth: size
                        Layout.maximumWidth: size

                        source: avatar ? "image://mxc/" + avatar : ""
                        name: model.name || i18n("No Name")
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignHCenter

                        spacing: Kirigami.Units.smallSpacing

                        Kirigami.Heading {
                            level: 3
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            text: name ?? ""
                            font.bold: unreadCount >= 0
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }

                        QQC2.Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.alignment: Qt.AlignHCenter

                            text: (lastEvent == "" ? topic : lastEvent).replace(/(\r\n\t|\n|\r\t)/gm," ")
                            visible: text.length > 0
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }
                    }
                    QQC2.Label {
                        text: notificationCount
                        visible: notificationCount > 0
                        padding: Kirigami.Units.smallSpacing
                        background: Rectangle {
                            Kirigami.Theme.colorSet: Kirigami.Theme.Button
                            color: Kirigami.Theme.backgroundColor
                            radius: height / 2
                            implicitWidth: implicitHeight
                        }
                    }
                }

                MouseArea {
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    anchors.fill: parent
                    onClicked: {
                        if (mouse.button == Qt.RightButton) {
                            roomListContextMenu.createObject(roomLayout, {"room": currentRoom}).popup()
                        } else {
                            enterRoomAction.trigger();
                        }
                    }
                }
            }
        }
        Component {
            id: roomListContextMenu
            RoomListContextMenu {}
        }
    }
}
