// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

Kirigami.OverlayDrawer {
    id: roomDrawer
    readonly property var room: RoomManager.currentRoom

    width: actualWidth

    readonly property int minWidth: Kirigami.Units.gridUnit * 15
    readonly property int maxWidth: Kirigami.Units.gridUnit * 25
    readonly property int defaultWidth: Kirigami.Units.gridUnit * 20
    property int actualWidth: {
        if (Config.roomDrawerWidth === -1) {
            return Kirigami.Units.gridUnit * 20;
        } else {
            return Config.roomDrawerWidth
        }
    }

    MouseArea {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: undefined
        width: 2
        z: 500
        cursorShape: !Kirigami.Settings.isMobile ? Qt.SplitHCursor : undefined
        enabled: true
        visible: true
        onPressed: _lastX = mapToGlobal(mouseX, mouseY).x
        onReleased: {
            Config.roomDrawerWidth = roomDrawer.actualWidth;
            Config.save();
        }
        property real _lastX: -1

        onPositionChanged: {
            if (_lastX === -1) {
                return;
            }
            if (Qt.application.layoutDirection === Qt.RightToLeft) {
                roomDrawer.actualWidth = Math.min(roomDrawer.maxWidth, Math.max(roomDrawer.minWidth, Config.roomDrawerWidth - _lastX + mapToGlobal(mouseX, mouseY).x))
            } else {
                roomDrawer.actualWidth = Math.min(roomDrawer.maxWidth, Math.max(roomDrawer.minWidth, Config.roomDrawerWidth + _lastX - mapToGlobal(mouseX, mouseY).x))
            }
        }
    }
    enabled: true

    edge: Qt.application.layoutDirection == Qt.RightToLeft ? Qt.LeftEdge : Qt.RightEdge

    // If modal has been changed and the drawer is closed automatically then dim on popup open will have been switched off in main.qml so switch it back on after the animation completes.
    // This is to avoid dim being active for a split second when the drawer is switched to modal which looks terrible.
    onAnimatingChanged: if (dim === false) dim = undefined

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    contentItem: Loader {
        id: loader
        active: roomDrawer.drawerOpen
        sourceComponent: ColumnLayout {
            id: columnLayout
            property alias userSearchText: userListSearchField.text
            property alias highlightedUser: userListView.currentIndex
            spacing: Kirigami.Units.largeSpacing

            Kirigami.AbstractApplicationHeader {
                Layout.fillWidth: true
                topPadding: Kirigami.Units.smallSpacing / 2;
                bottomPadding: Kirigami.Units.smallSpacing / 2;
                rightPadding: Kirigami.Units.largeSpacing
                leftPadding: Kirigami.Units.largeSpacing

                RowLayout {
                    anchors.fill: parent
                    Kirigami.Heading {
                        Layout.fillWidth: true
                        text: i18n("Room information")
                        level: 1
                    }
                    QQC2.ToolButton {
                        id: settingsButton

                        icon.name: "settings-configure"
                        text: i18n("Room settings")
                        display: QQC2.AbstractButton.IconOnly

                        onClicked: QQC2.ApplicationWindow.window.pageStack.pushDialogLayer('qrc:/Categories.qml', {room: room}, { title: i18n("Room Settings") })
                        QQC2.ToolTip {
                            text: settingsButton.text
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Avatar {
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 3.5
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 3.5

                    name: room ? room.displayName : ""
                    source: room ? ("image://mxc/" +  room.avatarMediaId) : ""

                    Rectangle {
                        visible: room.usesEncryption
                        color: Kirigami.Theme.backgroundColor

                        width: Kirigami.Units.gridUnit
                        height: Kirigami.Units.gridUnit
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right

                        radius: width / 2

                        Kirigami.Icon {
                            source: "channel-secure-symbolic"
                            anchors.fill: parent
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 0

                    Kirigami.Heading {
                        Layout.fillWidth: true
                        level: 1
                        type: Kirigami.Heading.Type.Primary
                        wrapMode: QQC2.Label.Wrap
                        text: room ? room.displayName : i18n("No name")
                        textFormat: Text.PlainText
                    }
                    Kirigami.SelectableLabel {
                        Layout.fillWidth: true
                        textFormat: TextEdit.PlainText
                        text: room && room.canonicalAlias ? room.canonicalAlias : i18n("No Canonical Alias")
                    }
                }
            }

            TextEdit {
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.fillWidth: true
                text: room && room.topic ? room.topic.replace(replaceLinks, "<a href=\"$1\">$1</a>") : i18n("No Topic")
                readonly property var replaceLinks: /(http[s]?:\/\/[^ \r\n]*)/g
                textFormat: TextEdit.MarkdownText
                wrapMode: Text.WordWrap
                selectByMouse: true
                color: Kirigami.Theme.textColor
                selectedTextColor: Kirigami.Theme.highlightedTextColor
                selectionColor: Kirigami.Theme.highlightColor
                onLinkActivated: UrlHelper.openUrl(link)
                readOnly: true
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                }
            }

            Kirigami.ListSectionHeader {
                label: i18n("Options")
                activeFocusOnTab: false
            }

            Kirigami.BasicListItem {
                id: devtoolsButton

                icon: "tools"
                text: i18n("Open developer tools")
                visible: Config.developerTools

                onClicked: {
                    applicationWindow().pageStack.layers.push("qrc:/DevtoolsPage.qml", {room: room}, {title: i18n("Developer Tools")})
                    roomDrawer.close();
                }
            }
            Kirigami.BasicListItem {
                id: searchButton

                icon: "search"
                text: i18n("Search in this room")

                onClicked: {
                    pageStack.pushDialogLayer("qrc:/SearchPage.qml", {
                        currentRoom: room
                    }, {
                        title: i18nc("@action:title", "Search")
                    })
                }
            }
            Kirigami.BasicListItem {
                id: locationsButton

                icon: "map-flat"
                text: i18n("Show locations for this room")

                onClicked: pageStack.pushDialogLayer("qrc:/LocationsPage.qml", {
                    room: room
                }, {
                    title: i18nc("Locations on a map", "Locations")
                })
            }
            Kirigami.BasicListItem {
                id: favouriteButton

                icon: room && room.isFavourite ? "rating" : "rating-unrated"
                text: room && room.isFavourite ? i18n("Remove room from favorites") : i18n("Make room favorite")

                onClicked: room.isFavourite ? room.removeTag("m.favourite") : room.addTag("m.favourite", 1.0)
            }

            Kirigami.ListSectionHeader {
                label: i18n("Members")
                activeFocusOnTab: false
                spacing: 0

                QQC2.ToolButton {
                    id: memberSearchToggle
                    checkable: true
                    icon.name: "search"
                    QQC2.ToolTip.text: i18n("Search user in room")
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                QQC2.ToolButton {
                    visible: roomDrawer.room.canSendState("invite")
                    icon.name: "list-add-user"

                    onClicked: {
                        applicationWindow().pageStack.layers.push("qrc:/InviteUserPage.qml", {room: roomDrawer.room})
                        roomDrawer.close();
                    }

                    QQC2.ToolTip.text: i18n("Invite user to room")
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                QQC2.Label {
                    Layout.alignment: Qt.AlignRight
                    text: room ? i18np("%1 Member", "%1 Members", room.joinedCount) : i18n("No Member Count")
                }
            }

            Kirigami.SearchField {
                id: userListSearchField
                visible: memberSearchToggle.checked

                onVisibleChanged: if (visible) forceActiveFocus()
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing - 1
                Layout.rightMargin: Kirigami.Units.largeSpacing - 1
                Layout.bottomMargin: Kirigami.Units.smallSpacing

                focusSequence: "Ctrl+Shift+F"

                onAccepted: sortedMessageEventModel.filterString = text;
            }

            QQC2.ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
                QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

                ListView {
                    id: userListView
                    clip: true
                    activeFocusOnTab: true

                    model: KSortFilterProxyModel {
                        id: sortedMessageEventModel

                        sourceModel: UserListModel {
                            room: roomDrawer.room
                        }

                        sortRole: "powerLevel"
                        sortOrder: Qt.DescendingOrder
                        filterRole: "name"
                        filterCaseSensitivity: Qt.CaseInsensitive
                    }

                    delegate: Kirigami.BasicListItem {
                        id: userListItem

                        implicitHeight: Kirigami.Units.gridUnit * 2
                        leftPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

                        label: name
                        labelItem.textFormat: Text.PlainText

                        onClicked: {
                            const popup = userDetailDialog.createObject(QQC2.ApplicationWindow.overlay, {room: room, user: user, displayName: name, avatarMediaId: avatar})
                            popup.closed.connect(function() {
                                userListItem.highlighted = false
                            })
                            if (roomDrawer.modal) {
                                roomDrawer.close()
                            }
                            popup.open()
                        }

                        leading: Kirigami.Avatar {
                            implicitWidth: height
                            sourceSize.height: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                            sourceSize.width: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                            source: avatar ? ("image://mxc/" + avatar) : ""
                            name: model.userId
                        }

                        trailing: QQC2.Label {
                            visible: powerLevel > 0

                            text: powerLevelString
                            color: Kirigami.Theme.disabledTextColor
                            textFormat: Text.PlainText
                            wrapMode: Text.NoWrap
                        }
                    }
                }
            }
        }
    }

    onRoomChanged: {
        if (loader.active) {
            loader.item.userSearchText = ""
            loader.item.highlightedUser = -1
        }
        if (room == null) {
            close()
        }
    }

    Component {
        id: userDetailDialog

        UserDetailDialog {}
    }
}
