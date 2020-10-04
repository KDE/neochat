import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.13 as Kirigami

import SortFilterProxyModel 0.2

import Spectral.Component 2.0
import Spectral 0.1

Kirigami.ScrollablePage {
    id: page

    property var roomListModel
    property var enteredRoom
    property var searchText

    signal enterRoom(var room)
    signal leaveRoom(var room)

    title: "Rooms"
    
    titleDelegate: Kirigami.SearchField {
        Layout.topMargin: Kirigami.Units.smallSpacing
        Layout.bottomMargin: Kirigami.Units.smallSpacing
        Layout.fillHeight: true
        Layout.fillWidth: true
        onTextChanged: page.searchText = text
    }

    ListView {
        id: messageListView

        model:  SortFilterProxyModel {
            sourceModel: roomListModel
            proxyRoles: ExpressionRole {
                name: "categoryName"
                expression: {
                    switch (category) {
                    case 1: return "Invited"
                    case 2: return "Favorites"
                    case 3: return "People"
                    case 4: return "Rooms"
                    case 5: return "Low Priority"
                    }
                }
            }

            sorters: [
                RoleSorter { roleName: "category" },
                ExpressionSorter {
                    expression: {
                        return modelLeft.highlightCount > 0;
                    }
                },
                ExpressionSorter {
                    expression: {
                        return modelLeft.notificationCount > 0;
                    }
                },
                RoleSorter {
                    roleName: "name"
                    sortOrder: Qt.DescendingOrder
                }
            ]

            filters: [
                ExpressionFilter {
                    expression: joinState != "upgraded"
                },
                RegExpFilter {
                    roleName: "name"
                    pattern: page.searchText
                    caseSensitivity: Qt.CaseInsensitive
                }
            ]
        }


        section.property: "categoryName"
        section.delegate: Kirigami.ListSectionHeader {
            label: section
        }

        delegate: Kirigami.AbstractListItem {
            topPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.largeSpacing

            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Avatar {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    source: avatar ? "image://mxc/" + avatar : ""
                    name: model.name || "No Name"
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

            onClicked: {
                if (enteredRoom) {
                    leaveRoom(enteredRoom)
                }

                enteredRoom = currentRoom

                enterRoom(enteredRoom)
            }
        }
    }
}
