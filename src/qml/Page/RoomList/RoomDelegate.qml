// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtQml.Models 2.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

import './' as RoomList

Kirigami.AbstractListItem {
    id: root

    required property int index
    required property int notificationCount
    required property int highlightCount
    required property var currentRoom
    required property bool categoryVisible
    required property string filterText
    required property string avatar
    required property string subtitleText

    required property string displayName
    readonly property bool hasNotifications: notificationCount > 0

    topPadding: Kirigami.Units.largeSpacing
    bottomPadding: Kirigami.Units.largeSpacing

    visible: root.categoryVisible || root.filterText.length > 0 || Config.mergeRoomList
    highlighted: ListView.view.currentIndex === index
    focus: true
    property bool bold: root.hasNotifications

    contentItem: Item {
        id: contItem

        implicitWidth: layout.implicitWidth

        Binding on implicitHeight {
            value: Math.max(iconItem.size, (!subtitleItem.visible && root.reserveSpaceForSubtitle ? (labelItem.implicitHeight + labelColumn.spacing + subtitleItem.implicitHeight): labelColumn.implicitHeight) )
            delayed: true
        }

        RowLayout {
            id: layout
            spacing: LayoutMirroring.enabled ? root.rightPadding : root.leftPadding
            anchors.left: contItem.left
            anchors.leftMargin: root.leading ? root.leadingPadding : 0
            anchors.right: contItem.right
            anchors.rightMargin: root.trailing ? root.trailingPadding : 0
            anchors.verticalCenter: parent.verticalCenter

            Kirigami.Avatar {
                source: root.avatar ? `image://mxc/${root.avatar}` : ""
                name: root.displayName
                implicitWidth: visible ? height : 0
                visible: Config.showAvatarInRoomDrawer
                Layout.preferredWidth: sourceSize.width
                Layout.preferredHeight: sourceSize.height
                sourceSize {
                    width: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                    height: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                }
            }

            Kirigami.Icon {
                id: iconItem
                source: root.icon.name !== "" ? root.icon.name : root.icon.source
                property int size: subtitleItem.visible || reserveSpaceForSubtitle ? Kirigami.Units.iconSizes.medium : Kirigami.Units.iconSizes.smallMedium
                Layout.minimumHeight: size
                Layout.maximumHeight: size
                Layout.minimumWidth: size
                Layout.maximumWidth: size
                selected: (root.highlighted || root.checked || root.down)
                opacity: root.fadeContent ? 0.6 : 1.0
                visible: source.toString() !== ""
            }
            ColumnLayout {
                id: labelColumn
                spacing: 0
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                TextEdit {
                    id: labelItem
                    text: labelMetrics.elidedText
                    Layout.fillWidth: true
                    Layout.alignment: subtitleItem.visible ? Qt.AlignLeft | Qt.AlignBottom : Qt.AlignLeft | Qt.AlignVCenter
                    color: (root.highlighted || root.checked || root.down) ? root.activeTextColor : root.textColor
                    font.weight: root.bold ? Font.Bold : Font.Normal
                    opacity: root.fadeContent ? 0.6 : 1.0
                    readOnly: true
                    Component.onCompleted: EmojiFixer.addTextDocument(labelItem.textDocument)
                    TextMetrics {
                        id: labelMetrics
                        font: labelItem.font
                        text: root.displayName
                        elideWidth: labelItem.width
                        elide: Qt.ElideRight
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: RoomManager.enterRoom(root.currentRoom)
                        onPressAndHold: createRoomListContextMenu()
                    }
                }
                TextEdit {
                    id: subtitleItem
                    Layout.fillWidth: true
                    Layout.alignment: subtitleItem.visible ? Qt.AlignLeft | Qt.AlignTop : Qt.AlignLeft | Qt.AlignVCenter
                    color: (root.highlighted || root.checked || root.down) ? root.activeTextColor : root.textColor
                    //elide: Text.ElideRight
                    font: Kirigami.Theme.smallFont
                    opacity: root.fadeContent ? 0.6 : (root.bold ? 0.9 : 0.7)
                    text: subtitleMetrics.elidedText
                    textFormat: Text.PlainText
                    visible: !Config.compactRoomList
                    readOnly: true
                    Component.onCompleted: EmojiFixer.addTextDocument(subtitleItem.textDocument)
                    TextMetrics {
                        id: subtitleMetrics
                        font: subtitleItem.font
                        text: root.subtitleText
                        elideWidth: subtitleItem.width
                        elide: Qt.ElideRight
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: RoomManager.enterRoom(root.currentRoom)
                        onPressAndHold: createRoomListContextMenu()
                    }
                }
            }
            Kirigami.Icon {
                source: "notifications-disabled"
                enabled: false
                implicitWidth: Kirigami.Units.iconSizes.smallMedium
                implicitHeight: Kirigami.Units.iconSizes.smallMedium
                visible: currentRoom.pushNotificationState === PushNotificationState.Mute && !configButton.visible && !hasNotifications
                Accessible.name: i18n("Muted room")
                Layout.rightMargin: Kirigami.Units.smallSpacing
            }
            QQC2.Label {
                id: notificationCountLabel
                text: notificationCount
                visible: hasNotifications
                color: Kirigami.Theme.textColor
                horizontalAlignment: Text.AlignHCenter
                background: Rectangle {
                    visible: notificationCount > 0
                    Kirigami.Theme.colorSet: Kirigami.Theme.Button
                    color: highlightCount > 0 ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
                    opacity: highlightCount > 0 ? 1 : 0.3
                    radius: height / 2
                }

                Layout.rightMargin: Kirigami.Units.smallSpacing
                Layout.minimumHeight: Kirigami.Units.iconSizes.smallMedium
                Layout.minimumWidth: Math.max(notificationCountTextMetrics.advanceWidth + Kirigami.Units.smallSpacing * 2, height)

                TextMetrics {
                    id: notificationCountTextMetrics
                    text: notificationCountLabel.text
                }
            }
            QQC2.Button {
                id: configButton
                visible: root.hovered && !Kirigami.Settings.isMobile && !Config.compactRoomList
                text: i18n("Configure room")
                display: QQC2.Button.IconOnly

                icon.name: "configure"
                onClicked: createRoomListContextMenu()
            }
            TapHandler {
                acceptedButtons: Qt.RightButton
                acceptedDevices: PointerDevice.Mouse
                onTapped: createRoomListContextMenu()
            }
        }
    }

    onClicked: RoomManager.enterRoom(root.currentRoom)
    onPressAndHold: createRoomListContextMenu()

    Keys.onEnterPressed: RoomManager.enterRoom(root.currentRoom)
    Keys.onReturnPressed: RoomManager.enterRoom(root.currentRoom)

    function createRoomListContextMenu() {
        const component = Qt.createComponent(Qt.resolvedUrl("./ContextMenu.qml"))
        const menu = component.createObject(root, {
            room: root.currentRoom,
        });
        if (!Kirigami.Settings.isMobile && !Config.compactRoomList) {
            configButton.visible = true;
            configButton.down = true;
        }
        menu.closed.connect(function() {
            configButton.down = undefined;
            configButton.visible = Qt.binding(() => {
                return root.hovered && !Kirigami.Settings.isMobile
                    && !Config.compactRoomList;
            });
        })
        menu.open()
    }
}
