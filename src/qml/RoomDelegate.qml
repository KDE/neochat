// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQml.Models

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.labs.components as Components
import org.kde.kitemmodels

import org.kde.neochat

Delegates.RoundedItemDelegate {
    id: root

    required property int index
    required property int contextNotificationCount
    required property bool hasHighlightNotifications
    required property NeoChatRoom currentRoom
    required property NeoChatConnection connection
    required property string avatar
    required property string subtitleText
    required property string displayName

    property bool showConfigure: true

    property bool collapsed: false

    readonly property bool hasNotifications: contextNotificationCount > 0

    Accessible.name: root.displayName
    Accessible.onPressAction: clicked()

    onClicked: {
        RoomManager.resolveResource(currentRoom.id);
        pageStack.currentIndex = 1;
    }

    onPressAndHold: createRoomListContextMenu()

    Keys.onSpacePressed: clicked()
    Keys.onEnterPressed: clicked()
    Keys.onReturnPressed: clicked()

    TapHandler {
        acceptedButtons: Qt.RightButton
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad | PointerDevice.Stylus
        onTapped: (eventPoint, button) => root.createRoomListContextMenu()
    }

    contentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        Components.Avatar {
            source: root.avatar ? root.connection.makeMediaUrl("mxc://" + root.avatar) : ""
            name: root.displayName
            visible: Config.showAvatarInRoomDrawer
            implicitHeight: Kirigami.Units.gridUnit + (Config.compactRoomList ? 0 : Kirigami.Units.largeSpacing * 2)
            implicitWidth: visible ? implicitHeight : 0

            Layout.fillHeight: true
            Layout.preferredWidth: height
        }

        ColumnLayout {
            spacing: 0

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            visible: !root.collapsed

            QQC2.Label {
                id: label

                text: root.displayName
                elide: Text.ElideRight
                font.weight: root.hasNotifications ? Font.Bold : Font.Normal
                textFormat: Text.PlainText

                Layout.fillWidth: true
                Layout.alignment: subtitle.visible ? Qt.AlignLeft | Qt.AlignBottom : Qt.AlignLeft | Qt.AlignVCenter
            }

            QQC2.Label {
                id: subtitle

                text: root.subtitleText
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
                opacity: root.hasNotifications ? 0.9 : 0.7
                visible: !Config.compactRoomList && text.length > 0
                textFormat: Text.PlainText

                Layout.fillWidth: true
                Layout.alignment: visible ? Qt.AlignLeft | Qt.AlignTop : Qt.AlignLeft | Qt.AlignVCenter
            }
        }

        Kirigami.Icon {
            source: "notifications-disabled"
            enabled: false
            implicitWidth: Kirigami.Units.iconSizes.smallMedium
            implicitHeight: Kirigami.Units.iconSizes.smallMedium
            visible: currentRoom.pushNotificationState === PushNotificationState.Mute && !configButton.visible && !root.collapsed
            Accessible.name: i18n("Muted room")
            Layout.rightMargin: Kirigami.Units.smallSpacing
        }

        QQC2.Label {
            id: notificationCountLabel

            text: root.contextNotificationCount
            visible: root.hasNotifications && !root.collapsed
            color: Kirigami.Theme.textColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            background: Rectangle {
                visible: root.hasNotifications
                Kirigami.Theme.colorSet: Kirigami.Theme.Button
                Kirigami.Theme.inherit: false
                color: root.hasHighlightNotifications > 0 ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.backgroundColor
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
            visible: root.hovered && !Kirigami.Settings.isMobile && !Config.compactRoomList && !root.collapsed && root.showConfigure
            text: i18n("Configure room")
            display: QQC2.Button.IconOnly

            icon.name: "configure"
            onClicked: createRoomListContextMenu()
        }
    }

    function createRoomListContextMenu() {
        const component = Qt.createComponent('org.kde.neochat', 'ContextMenu');
        if (component.status === Component.Error) {
            console.error(component.errorString());
        }
        const menu = component.createObject(root, {
            room: root.currentRoom,
            connection: root.connection
        });
        if (!Kirigami.Settings.isMobile && !Config.compactRoomList) {
            configButton.visible = true;
            configButton.down = true;
        }
        menu.closed.connect(function () {
            configButton.down = undefined;
            configButton.visible = Qt.binding(() => {
                return root.hovered && !Kirigami.Settings.isMobile && !Config.compactRoomList;
            });
        });
        menu.open();
    }
}
