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
    Layout.minimumHeight: bottomEdge ? Kirigami.Units.gridUnit * 2 : -1

    onVisibleChanged: {
        if (!visible) {
            accountsPopup.close();
            switchUserButton.checked = false;
        }
    }
    KirigamiComponents.AvatarButton {
        id: accountButton
        readonly property string mediaId: root.connection.localUser.avatarMediaId

        Layout.preferredWidth: Kirigami.Units.iconSizes.medium
        Layout.preferredHeight: Kirigami.Units.iconSizes.medium
        Layout.leftMargin: Kirigami.Units.largeSpacing

        text: i18n("Edit this account")
        source: mediaId ? root.connection.makeMediaUrl("mxc://" + mediaId) : ""

        activeFocusOnTab: true

        onClicked: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.settings', 'NeoChatSettings'), {
            defaultPage: "accounts",
            connection: root.connection,
            initialAccount: root.connection
        }, {
            title: i18nc("@action:button", "Configure"),
            width: Kirigami.Units.gridUnit * 50,
            height: Kirigami.Units.gridUnit * 42
        })

        TapHandler {
            acceptedButtons: Qt.RightButton
            onTapped:  accountMenu.open()
        }
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
    Kirigami.ActionToolBar {
        alignment: Qt.AlignRight
        display: QQC2.Button.IconOnly

        actions: [
            Kirigami.Action {
                id: switchUserButton
                text: i18n("Switch User")
                icon.name: "system-switch-user"
                onTriggered: accountSwitchDialog.createObject(QQC2.ApplicationWindow.overlay, {
                    connection: root.connection
                }).open();
            },
            Kirigami.Action {
                text: i18n("Open Settings")
                icon.name: "settings-configure"
                onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.settings', 'NeoChatSettings'), {
                    connection: root.connection
                }, {
                    title: i18n("Configure"),
                    width: Kirigami.Units.gridUnit * 50,
                    height: Kirigami.Units.gridUnit * 42
                })
            }
        ]

        Shortcut {
            sequence: "Ctrl+U"
            onActivated: switchUserButton.toggle()
        }
    }
    // QQC2.ToolButton {
    //     Layout.alignment: Qt.AlignRight
    //     display: QQC2.AbstractButton.IconOnly
    //     action: Kirigami.Action {
    //         id: switchUserButton
    //         text: i18n("Switch User")
    //         icon.name: "system-switch-user"
    //         onTriggered: accountSwitchDialog.createObject(QQC2.ApplicationWindow.overlay, {
    //             connection: root.connection
    //         }).open();
    //     }
    //     QQC2.ToolTip.text: text
    //     QQC2.ToolTip.visible: hovered
    //     QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    //     Shortcut {
    //         sequence: "Ctrl+U"
    //         onActivated: switchUserButton.trigger()
    //     }
    // }
    // QQC2.ToolButton {
    //     Layout.alignment: Qt.AlignRight
    //     display: QQC2.AbstractButton.IconOnly
    //     action: Kirigami.Action {
    //         text: i18n("Open Settings")
    //         icon.name: "settings-configure"
    //         onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.settings', 'NeoChatSettings.qml'), {
    //             connection: root.connection
    //         }, {
    //             title: i18n("Configure"),
    //             width: Kirigami.Units.gridUnit * 50,
    //             height: Kirigami.Units.gridUnit * 42
    //         })
    //     }
    //     QQC2.ToolTip.text: text
    //     QQC2.ToolTip.visible: hovered
    //     QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    // }

    AccountMenu {
        id: accountMenu
        y: root.bottomEdge ? -height : accountButton.height
        connection: root.connection
    }
    Component {
        id: accountSwitchDialog
        AccountSwitchDialog {}
    }
}
