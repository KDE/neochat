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

Kirigami.BasicListItem {
    id: root

    required property int index
    required property int notificationCount
    required property int highlightCount
    required property NeoChatRoom currentRoom
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
    icon: undefined
    @BASICLISTITEM_BOLD@: root.hasNotifications

    label: root.displayName
    labelItem.textFormat: Text.PlainText

    subtitle: root.subtitleText
    subtitleItem {
        textFormat: Text.PlainText
        visible: !Config.compactRoomList
    }

    onClicked: RoomManager.enterRoom(root.currentRoom)
    onPressAndHold: createRoomListContextMenu()

    Keys.onEnterPressed: RoomManager.enterRoom(root.currentRoom)
    Keys.onReturnPressed: RoomManager.enterRoom(root.currentRoom)

    TapHandler {
        acceptedButtons: Qt.RightButton
        acceptedDevices: PointerDevice.Mouse
        onTapped: createRoomListContextMenu()
    }

    leading: Kirigami.Avatar {
        source: root.avatar ? `image://mxc/${root.avatar}` : ""
        name: root.displayName
        implicitWidth: visible ? height : 0
        visible: Config.showAvatarInRoomDrawer
        sourceSize {
            width: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
            height: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
        }
    }

    trailing: RowLayout {
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
    }

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
