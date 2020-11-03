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
import Spectral.Component 2.0
import Spectral 0.1

Kirigami.ScrollablePage {
    id: page

    property var roomListModel
    property var enteredRoom
    property var searchText: ""

    onSearchTextChanged: sortedFilteredRoomListModel.invalidateFilter()

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
        model:  KSortFilterProxyModel {
            id: sortedFilteredRoomListModel
            sourceModel: roomListModel

            sortRole: "name"
            sortOrder: Qt.AscendingOrder
            filterRowCallback: function(row, parent) {
                return (roomListModel.data(roomListModel.index(row, 0), RoomListModel.JoinStateRole) !== "upgraded") && roomListModel.data(roomListModel.index(row, 0), RoomListModel.NameRole).toLowerCase().includes(page.searchText.toLowerCase())
            }
        }

        section.property: "categoryName"
        section.delegate: Kirigami.ListSectionHeader {
            label: section
        }

        delegate: Kirigami.AbstractListItem {
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
