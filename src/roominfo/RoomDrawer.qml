// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kitemmodels

import org.kde.neochat.libneochat
import org.kde.neochat.timeline as Timeline
import org.kde.neochat.settings as Settings

Kirigami.OverlayDrawer {
    id: root

    required property NeoChatRoom room
    required property NeoChatConnection connection
    required property UserListModel userListModel
    required property Timeline.MediaMessageFilterModel mediaMessageFilterModel

    signal resolveResource(string idOrUri, string action)

    width: actualWidth
    interactive: modal

    readonly property int minWidth: Kirigami.Units.gridUnit * 15
    readonly property int maxWidth: Kirigami.Units.gridUnit * 25
    readonly property int defaultWidth: Kirigami.Units.gridUnit * 20
    property int roomDrawerWidth
    property int actualWidth: {
        if (root.roomDrawerWidth === -1) {
            return Kirigami.Units.gridUnit * 20;
        } else {
            return root.roomDrawerWidth;
        }
    }

    onOpened: forceActiveFocus()

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
            root.roomDrawerWidth = root.actualWidth;
        }
        property real _lastX: -1

        onPositionChanged: {
            if (_lastX === -1) {
                return;
            }
            if (Qt.application.layoutDirection === Qt.RightToLeft) {
                root.actualWidth = Math.min(root.maxWidth, Math.max(root.minWidth, root.roomDrawerWidth - _lastX + mapToGlobal(mouseX, mouseY).x));
            } else {
                root.actualWidth = Math.min(root.maxWidth, Math.max(root.minWidth, root.roomDrawerWidth + _lastX - mapToGlobal(mouseX, mouseY).x));
            }
        }
    }
    enabled: true

    edge: Qt.application.layoutDirection == Qt.RightToLeft ? Qt.LeftEdge : Qt.RightEdge

    // If modal has been changed and the drawer is closed automatically then dim on popup open will have been switched off in main.qml so switch it back on after the animation completes.
    // This is to avoid dim being active for a split second when the drawer is switched to modal which looks terrible.
    onAnimatingChanged: if (dim === false)
        dim = undefined

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    Kirigami.Theme.colorSet: Kirigami.Theme.View

    contentItem: Loader {
        id: loader
        active: root.drawerOpen

        sourceComponent: RowLayout {
            spacing: 0

            Kirigami.Separator {
                Layout.fillHeight: true
                visible: root.modal
            }

            ColumnLayout {
                spacing: 0

                Component.onCompleted: infoAction.toggle()

                QQC2.ToolBar {
                    Layout.fillWidth: true

                    Layout.preferredHeight: pageStack.globalToolBar.preferredHeight

                    contentItem: RowLayout {
                        spacing: 0

                        Kirigami.Heading {
                            Layout.fillWidth: true
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            text: drawerItemLoader.item ? drawerItemLoader.item.title : ""
                        }

                        QQC2.ToolButton {
                            id: settingsButton

                            display: QQC2.AbstractButton.IconOnly
                            text: i18nc("@action:button", "Room settings")
                            icon.name: 'settings-configure-symbolic'

                            QQC2.ToolTip.text: text
                            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                            QQC2.ToolTip.visible: hovered

                            onClicked: {
                                Settings.RoomSettingsView.openRoomSettings(root.room, Settings.RoomSettingsView.Room);
                            }
                        }
                    }
                }

                Loader {
                    id: drawerItemLoader
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    sourceComponent: roomInformation
                }

                Component {
                    id: roomInformation
                    RoomInformation {
                        room: root.room
                        userListModel: root.userListModel

                        onResolveResource: (idOrUri, action) => root.resolveResource(idOrUri, action)
                    }
                }

                Component {
                    id: roomMedia
                    RoomMedia {
                        room: root.room
                        mediaMessageFilterModel: root.mediaMessageFilterModel
                    }
                }

                Kirigami.NavigationTabBar {
                    id: navigationBar
                    Layout.fillWidth: true
                    visible: !root.room.isSpace
                    Kirigami.Theme.colorSet: Kirigami.Theme.Window
                    Kirigami.Theme.inherit: false

                    position: QQC2.ToolBar.Footer

                    actions: [
                        Kirigami.Action {
                            id: infoAction
                            text: i18n("Information")
                            icon.name: "documentinfo"
                            onTriggered: drawerItemLoader.sourceComponent = roomInformation
                        },
                        Kirigami.Action {
                            text: i18n("Media")
                            icon.name: "mail-attachment-symbollic"
                            onTriggered: drawerItemLoader.sourceComponent = roomMedia
                        }
                    ]
                }
            }
        }
    }
}
