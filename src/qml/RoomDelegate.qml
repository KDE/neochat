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
import org.kde.neochat.config

Delegates.RoundedItemDelegate {
    id: root

    required property int index
    required property int notificationCount
    required property int highlightCount
    required property NeoChatRoom currentRoom
    required property NeoChatConnection connection
    required property bool categoryVisible
    required property string filterText
    required property string avatar
    required property string subtitleText

    required property string displayName

    readonly property bool hasNotifications: currentRoom.pushNotificationState === PushNotificationState.MentionKeyword || currentRoom.isLowPriority ? highlightCount > 0 : notificationCount > 0

    signal selected

    Accessible.name: root.displayName
    Accessible.onPressAction: select()

    onPressAndHold: createRoomListContextMenu()

    Keys.onSpacePressed: select()
    Keys.onEnterPressed: select()
    Keys.onReturnPressed: select()

    TapHandler {
        acceptedButtons: Qt.RightButton | Qt.LeftButton
        onTapped: (eventPoint, button) => {
            if (button === Qt.RightButton) {
                root.createRoomListContextMenu();
            } else {
                select();
            }
        }
    }

    contentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        Components.Avatar {
            source: root.avatar ? "image://mxc/" + root.avatar : ""
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
            visible: currentRoom.pushNotificationState === PushNotificationState.Mute && !configButton.visible
            Accessible.name: i18n("Muted room")
            Layout.rightMargin: Kirigami.Units.smallSpacing
        }

        QQC2.Label {
            id: notificationCountLabel

            text: currentRoom.pushNotificationState === PushNotificationState.MentionKeyword || currentRoom.isLowPriority ? root.highlightCount : root.notificationCount
            visible: root.hasNotifications && currentRoom.pushNotificationState !== PushNotificationState.Mute
            color: Kirigami.Theme.textColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            background: Rectangle {
                visible: root.hasNotifications
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

    function select() {
        RoomManager.resolveResource(currentRoom.id);
        root.selected();
    }

    function createRoomListContextMenu() {
        const component = Qt.createComponent("qrc:/org/kde/neochat/qml/ContextMenu.qml");
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
