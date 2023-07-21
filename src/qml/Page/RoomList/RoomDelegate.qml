// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtQml.Models 2.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.delegates 1.0 as Delegates
import org.kde.kirigamiaddons.labs.components 1.0 as Components
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

import './' as RoomList

Delegates.RoundedItemDelegate {
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

    onPressAndHold: createRoomListContextMenu()

    TapHandler {
        acceptedButtons: Qt.RightButton
        acceptedDevices: PointerDevice.Mouse
        onTapped: createRoomListContextMenu()
    }

    contentItem: RowLayout {
        Components.Avatar {
            source: root.avatar ? "image://mxc/" +  root.avatar : ""
            name: root.displayName
            implicitWidth: visible ? height : 0
            implicitHeight: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
            visible: Config.showAvatarInRoomDrawer
            sourceSize {
                width: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
                height: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
            }

            Layout.topMargin: Kirigami.Units.largeSpacing / 2
            Layout.bottomMargin: Kirigami.Units.largeSpacing / 2
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

                Layout.fillWidth: true
                Layout.alignment: subtitle.visible ? Qt.AlignLeft | Qt.AlignBottom : Qt.AlignLeft | Qt.AlignVCenter
            }

            QQC2.Label {
                id: subtitle

                text: root.subtitleText
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
                opacity: root.hasNotifications ? 0.9 : 0.7
                visible: !Config.compactRoomList
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
            visible: currentRoom.pushNotificationState === PushNotificationState.Mute && !configButton.visible && !hasNotifications
            Accessible.name: i18n("Muted room")
            Layout.rightMargin: Kirigami.Units.smallSpacing
        }

        QQC2.Label {
            id: notificationCountLabel

            text: root.notificationCount
            visible: root.hasNotifications
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
