// SPDX-FileCopyrightText: 2020-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2021-2022 Bart De Vries <bart@mogwai.be>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

import '.'
import org.kde.neochat 1.0

QQC2.Control {
    id: root

    readonly property real pinnedWidth: Kirigami.Units.gridUnit * 6
    readonly property int buttonDisplayMode: Kirigami.NavigationTabButton.IconOnly
    property bool drawerEnabled: true

    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    Kirigami.Theme.inherit: false

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    property string selectedSpaceId

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
    }

    contentItem: Loader {
        id: sidebarColumn
        active: root.drawerEnabled
        z: 0

        sourceComponent: ColumnLayout {
            spacing: 0

            QQC2.ScrollView {
                id: scrollView
                Layout.fillWidth: true
                Layout.fillHeight: true

                QQC2.ScrollBar.vertical.policy: QQC2.ScrollBar.AlwaysOff
                QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff
                contentWidth: -1 // disable horizontal scroll

                ColumnLayout {
                    id: column
                    width: scrollView.width
                    spacing: 0

                    Kirigami.NavigationTabButton {
                        Layout.fillWidth: true
                        Layout.preferredHeight: width
                        display: Kirigami.NavigationTabButton.IconOnly
                        text: i18n("All Rooms")
                        icon.name: "globe"
                        checked: true
                        onClicked: root.selectedSpaceId = ""
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }

                    Repeater {
                        model: SortFilterSpaceListModel {
                            sourceModel: RoomListModel {
                                connection: Controller.activeConnection
                            }
                        }
                        onCountChanged: root.enabled = count > 0

                        delegate: AvatarTabButton {
                            Layout.fillWidth: true
                            Layout.preferredHeight: width
                            display: Kirigami.NavigationTabButton.IconOnly
                            text: model.name
                            source: model.avatar ? ("image://mxc/" + model.avatar) : ""
                            name: model.name
                            onClicked: root.selectedSpaceId = model.id
                            QQC2.ToolTip.visible: hovered
                            QQC2.ToolTip.text: text
                            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                            onPressAndHold: root.createContextMenu(model.currentRoom)

                            TapHandler {
                                acceptedButtons: Qt.RightButton
                                acceptedDevices: PointerDevice.Mouse
                                onTapped: root.createContextMenu(model.currentRoom)
                            }
                        }
                    }
                }
            }
        }
    }

    function createContextMenu(room) {
        let context = spaceListContextMenu.createObject(root, {
            room: room
        });
        context.open()
    }
    Component {
        id: spaceListContextMenu
        SpaceListContextMenu {}
    }
}
