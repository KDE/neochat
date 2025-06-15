// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat
import org.kde.neochat.settings

RowLayout {
    id: root

    required property NeoChatConnection connection

    property bool collapsed: false

    property bool bottomEdge: true

    property var addAccount

    spacing: Kirigami.Units.largeSpacing

    Layout.topMargin: Kirigami.Units.smallSpacing
    Layout.bottomMargin: Kirigami.Units.smallSpacing
    Layout.rightMargin: Kirigami.Units.largeSpacing
    Layout.leftMargin: Kirigami.Units.largeSpacing
    Layout.minimumHeight: bottomEdge ? Kirigami.Units.gridUnit * 3 : -1

    onVisibleChanged: {
        if (!visible) {
            accountsPopup.close();
        }
    }

    QQC2.ToolButton {
        id: accountButton

        down: accountMenu.opened || pressed

        onClicked: accountMenu.popup()

        Layout.fillWidth: true
        Layout.fillHeight: true

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: i18nc("@info:tooltip", "Manage Account")
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

        contentItem: RowLayout {
            spacing: Kirigami.Units.largeSpacing

            KirigamiComponents.Avatar {
                readonly property url avatarUrl: root.connection.localUser.avatarUrl

                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                Layout.leftMargin: Kirigami.Units.largeSpacing

                // Note: User::avatarUrl does not set user_id, and thus cannot be used directly here. Hence the makeMediaUrl.
                source: avatarUrl.toString().length > 0 ? root.connection.makeMediaUrl(avatarUrl) : ""
                name: root.connection.localUser.displayName
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.maximumWidth: Math.round(root.width * 0.55)
                visible: !root.collapsed
                spacing: 0
                QQC2.Label {
                    id: displayNameLabel
                    Layout.fillWidth: true
                    text: root.connection.localUser.displayName
                    textFormat: Text.PlainText
                    elide: Text.ElideRight
                }
                QQC2.Label {
                    id: idLabel
                    Layout.fillWidth: true
                    text: (root.connection.label.length > 0 ? (root.connection.label + " ") : "") + root.connection.localUser.id
                    font.pointSize: displayNameLabel.font.pointSize * 0.8
                    opacity: 0.7
                    textFormat: Text.PlainText
                    elide: Text.ElideRight
                }
            }
        }

        AccountMenu {
            id: accountMenu
            connection: root.connection
            window: QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow
        }
    }

    Kirigami.ActionToolBar {
        alignment: Qt.AlignRight
        display: QQC2.Button.IconOnly

        Layout.fillWidth: true
        Layout.preferredWidth: maximumContentWidth

        actions: [
            Kirigami.Action {
                text: i18n("Open Settings")
                icon.name: "settings-configure-symbolic"
                onTriggered: {
                    NeoChatSettingsView.open();
                }
            }
        ]
    }

    Component {
        id: accountSwitchDialog
        AccountSwitchDialog {}
    }
}
