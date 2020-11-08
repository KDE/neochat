/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.13 as Kirigami
import org.kde.kitemmodels 1.0
import org.kde.neochat 0.1

import NeoChat.Component 2.0
import NeoChat.Menu 2.0

Kirigami.ScrollablePage {
    id: page

    property var roomListModel
    property var enteredRoom
    property var searchText: ""

    onSearchTextChanged: sortFilterRoomListModel.setFilterText(searchText)

    signal enterRoom(var room)
    signal leaveRoom(var room)

    title: i18n("Rooms")

    titleDelegate: Kirigami.SearchField {
        Layout.topMargin: Kirigami.Units.smallSpacing
        Layout.bottomMargin: Kirigami.Units.smallSpacing
        Layout.fillHeight: true
        Layout.fillWidth: true
        onTextChanged: page.searchText = text
    }

    ListView {
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

        delegate: Kirigami.AbstractListItem {
            visible: model.categoryVisible
            topPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.largeSpacing

            contentItem: Item {
                implicitHeight: roomLayout.implicitHeight
                RowLayout {
                    id: roomLayout
                    spacing: Kirigami.Units.largeSpacing
                    anchors.fill: parent

                    Kirigami.Avatar {
                        Layout.preferredWidth: height
                        Layout.fillHeight: true

                        source: avatar ? "image://mxc/" + avatar : ""
                        name: model.name || i18n("No Name")
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignHCenter

                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            text: name ?? ""
                            font.pixelSize: 15
                            font.bold: unreadCount >= 0
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }

                        QQC2.Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.alignment: Qt.AlignHCenter

                            text: (lastEvent == "" ? topic : lastEvent).replace(/(\r\n\t|\n|\r\t)/gm," ")
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }
                    }
                }
                MouseArea {
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    anchors.fill: parent
                    onClicked: {
                        console.log(mouse.button)
                        if (mouse.button == Qt.RightButton) {
                            roomListContextMenu.createObject(parent, {"room": currentRoom}).popup()
                        } else {
                            if (enteredRoom) {
                                leaveRoom(enteredRoom)
                            }

                            enteredRoom = currentRoom

                            enterRoom(enteredRoom)
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
