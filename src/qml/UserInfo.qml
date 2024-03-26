// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat
import org.kde.neochat.settings
import org.kde.neochat.config
import org.kde.neochat.accounts

RowLayout {
    id: root

    required property NeoChatConnection connection

    property bool collapsed: false

    property bool bottomEdge: true

    property var addAccount

    spacing: Kirigami.Units.largeSpacing

    Layout.topMargin: Kirigami.Units.smallSpacing
    Layout.bottomMargin: Kirigami.Units.smallSpacing
    Layout.minimumHeight: bottomEdge ? Kirigami.Units.gridUnit * 2 : -1

    onVisibleChanged: {
        if (!visible) {
            accountsPopup.close();
            switchUserButton.checked = false;
        }
    }

    QQC2.AbstractButton {
        id: accountButton

        Layout.preferredWidth: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
        Layout.preferredHeight: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
        Layout.leftMargin: Kirigami.Units.largeSpacing

        TapHandler {
            acceptedButtons: Qt.RightButton | Qt.LeftButton
            onTapped: (eventPoint, button) => {
                // TODO Qt6 remove
                if (!button) {
                    button = eventPoint.event.button;
                }
                if (button == Qt.RightButton) {
                    accountMenu.open();
                } else {
                    pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.settings', 'AccountEditorPage.qml'), {
                        connection: root.connection
                    }, {
                        title: i18n("Account editor")
                    });
                }
            }
        }

        text: i18n("Edit this account")

        contentItem: KirigamiComponents.Avatar {
            readonly property string mediaId: root.connection.localUser.avatarMediaId

            source: mediaId ? ("image://mxc/" + mediaId) : ""
            name: root.connection.localUser.displayName ?? root.connection.localUser.id
        }
    }

    ColumnLayout {
        visible: !root.collapsed
        spacing: 0
        QQC2.Label {
            id: displayNameLabel
            text: root.connection.localUser.displayName
            textFormat: Text.PlainText
            elide: Text.ElideRight
        }
        QQC2.Label {
            text: (root.connection.label.length > 0 ? (root.connection.label + " ") : "") + root.connection.localUser.id
            font.pointSize: displayNameLabel.font.pointSize * 0.8
            opacity: 0.7
            textFormat: Text.PlainText
            elide: Text.ElideRight
        }
    }
    Kirigami.ActionToolBar {
        alignment: Qt.AlignRight
        rightPadding: root.collapsed ? Kirigami.Units.largeSpacing : 0
        display: QQC2.Button.IconOnly

        actions: [
            Kirigami.Action {
                id: switchUserButton
                text: i18n("Switch User")
                icon.name: "system-switch-user"
                checkable: !root.collapsed
                onTriggered: if (root.collapsed) {
                    accountSwitchDialog.createObject(QQC2.ApplicationWindow.overlay, {
                        connection: root.connection
                    }).open();
                }
            },
            Kirigami.Action {
                visible: Config.developerTools
                text: i18n("Open developer tools")
                icon.name: "tools"
                onTriggered: applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'DevtoolsPage.qml'), {
                    connection: root.connection
                }, {
                    title: i18n("Developer Tools")
                });
            },
            Kirigami.Action {
                visible: root.collapsed
                text: i18n("Open Settings")
                icon.name: "settings-configure"
                onTriggered: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.settings', 'NeoChatSettings.qml'), {
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
    QQC2.ToolButton {
        visible: !root.collapsed
        icon.name: "settings-configure"
        onClicked: pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.settings', 'NeoChatSettings.qml'), {
            connection: root.connection
        }, {
            title: i18n("Configure"),
            width: Kirigami.Units.gridUnit * 50,
            height: Kirigami.Units.gridUnit * 42
        })
        text: i18n("Open Settings")
        display: QQC2.AbstractButton.IconOnly
        Layout.minimumWidth: Layout.preferredWidth
        Layout.alignment: Qt.AlignRight
        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
    Item {
        visible: !root.collapsed
        width: 1
    }

    AccountMenu {
        id: accountMenu
        y: root.bottomEdge ? -height : accountButton.height
        connection: root.connection
    }
    QQC2.Popup {
        id: accountsPopup
        parent: root

        visible: switchUserButton.checked
        onVisibleChanged: if (visible) {
            accounts.forceActiveFocus()
        }

        x: -Kirigami.Units.smallSpacing
        y: root.bottomEdge ? -height - Kirigami.Units.smallSpacing - 1 : root.height + Kirigami.Units.smallSpacing - 1
        width: root.width + (root.bottomEdge ? 0 : Kirigami.Units.smallSpacing * 2)
        leftPadding: 0
        rightPadding: 0
        bottomPadding: Kirigami.Units.smallSpacing
        topPadding: Kirigami.Units.smallSpacing

        closePolicy: QQC2.Popup.CloseOnEscape

        contentItem: AccountView {
            id: accounts
            connection: root.connection
        }

        background: ColumnLayout {
            spacing: 0
            Kirigami.Separator {
                Layout.fillWidth: true
                visible: root.bottomEdge
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Kirigami.Theme.backgroundColor
            }
            Kirigami.Separator {
                Layout.fillWidth: true
                visible: !root.bottomEdge
            }
        }
    }

    Component {
        id: accountSwitchDialog
        AccountSwitchDialog {}
    }
}
