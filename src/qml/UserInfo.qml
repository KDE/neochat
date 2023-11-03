// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat
import org.kde.neochat.config
import org.kde.neochat.accounts

RowLayout {
    id: root

    required property NeoChatConnection connection

    property bool bottomEdge: true

    property var addAccount

    spacing: Kirigami.Units.largeSpacing

    Layout.topMargin: Kirigami.Units.smallSpacing
    Layout.bottomMargin: Kirigami.Units.smallSpacing
    Layout.minimumHeight: bottomEdge ? Kirigami.Units.gridUnit * 2 - 2 : -1 // HACK: -2 here is to ensure the ChatBox and the UserInfo have the same height

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
                    pageStack.pushDialogLayer(Qt.resolvedUrl('qrc:/org/kde/neochat/qml/AccountEditorPage.qml'), {
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
        Layout.fillWidth: true
        spacing: 0
        QQC2.Label {
            id: displayNameLabel
            text: root.connection.localUser.displayName
            textFormat: Text.PlainText
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
        QQC2.Label {
            text: (root.connection.label.length > 0 ? (root.connection.label + " ") : "") + root.connection.localUser.id
            font.pointSize: displayNameLabel.font.pointSize * 0.8
            opacity: 0.7
            textFormat: Text.PlainText
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
    }
    QQC2.ToolButton {
        id: switchUserButton
        icon.name: "system-switch-user"
        checkable: true
        text: i18n("Switch User")
        display: QQC2.AbstractButton.IconOnly
        Accessible.name: text
        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        Layout.minimumWidth: Layout.preferredWidth
        Layout.alignment: Qt.AlignRight
        Shortcut {
            sequence: "Ctrl+U"
            onActivated: switchUserButton.toggle()
        }
    }
    QQC2.ToolButton {
        icon.name: "list-add"
        onClicked: ; //TODO
        text: i18n("Add") //TODO find better message
        display: QQC2.AbstractButton.IconOnly
        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        Layout.minimumWidth: Layout.preferredWidth
        Layout.alignment: Qt.AlignRight
        visible: false
    }
    QQC2.ToolButton {
        icon.name: "settings-configure"
        onClicked: pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/SettingsPage.qml", {connection: root.connection}, { title: i18n("Configure") })
        text: i18n("Open Settings")
        display: QQC2.AbstractButton.IconOnly
        Layout.minimumWidth: Layout.preferredWidth
        Layout.alignment: Qt.AlignRight
        Layout.rightMargin: Kirigami.Units.largeSpacing
        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
    Item {
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
        onVisibleChanged: if (visible) accounts.forceActiveFocus()

        x: -Kirigami.Units.smallSpacing
        y: root.bottomEdge ? -height - Kirigami.Units.smallSpacing - 1  : root.height + Kirigami.Units.smallSpacing - 1
        width: root.width + (root.bottomEdge ? 0 : Kirigami.Units.smallSpacing * 2)
        leftPadding: 0
        rightPadding: 0
        bottomPadding: Kirigami.Units.smallSpacing
        topPadding: Kirigami.Units.smallSpacing

        closePolicy: QQC2.Popup.CloseOnEscape

        contentItem: ListView {
            id: accounts
            implicitHeight: contentHeight

            currentIndex: Controller.activeConnectionIndex

            header: Kirigami.Separator {}

            footer: Delegates.RoundedItemDelegate {
                id: addButton
                width: parent.width
                highlighted: focus || (addAccount.highlighted || addAccount.ListView.isCurrentItem) && !addAccount.pressed
                Component.onCompleted: root.addAccount = this
                icon {
                    name: "list-add"
                    width: Kirigami.Units.iconSizes.smallMedium
                    height: Kirigami.Units.iconSizes.smallMedium
                }
                text: i18n("Add Account")
                contentItem: Delegates.SubtitleContentItem {
                    itemDelegate: parent
                    subtitle: i18n("Log in to an existing account")
                    labelItem.textFormat: Text.PlainText
                    subtitleItem.textFormat: Text.PlainText
                }

                onClicked: {
                    pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/WelcomePage.qml", {}, {
                        title: i18nc("@title:window", "Login"),
                    });
                    if (switchUserButton.checked) {
                        switchUserButton.checked = false
                    }
                    accounts.currentIndex = Controller.activeConnectionIndex
                }
                Keys.onUpPressed: {
                    accounts.currentIndex = accounts.count - 1
                    accounts.forceActiveFocus()
                }
                Keys.onDownPressed: {
                    accounts.currentIndex = 0
                    accounts.forceActiveFocus()
                }
            }
            clip: true
            model: AccountRegistry

            keyNavigationEnabled: false
            Keys.onDownPressed: {
                if (accounts.currentIndex === accounts.count - 1) {
                    addAccount.forceActiveFocus()
                    accounts.currentIndex = -1
                } else {
                    accounts.incrementCurrentIndex()
                }
            }
            Keys.onUpPressed: {
                if (accounts.currentIndex === 0) {
                    addAccount.forceActiveFocus()
                    accounts.currentIndex = -1
                } else {
                    accounts.decrementCurrentIndex()
                }
            }

            Keys.onReleased: if (event.key == Qt.Key_Escape) {
                if (switchUserButton.checked) {
                    switchUserButton.checked = false
                }
            }

            delegate: Delegates.RoundedItemDelegate {
                id: userDelegate

                required property NeoChatConnection connection

                width: parent.width
                text: connection.localUser.displayName

                contentItem: RowLayout {
                    KirigamiComponents.Avatar {
                        implicitWidth: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
                        implicitHeight: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
                        sourceSize {
                            width: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
                            height: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
                        }
                        source: userDelegate.connection.localUser.avatarMediaId ? ("image://mxc/" + userDelegate.connection.localUser.avatarMediaId) : ""
                        name: userDelegate.connection.localUser.displayName ?? userDelegate.connection.localUser.id
                    }

                    Delegates.SubtitleContentItem {
                        itemDelegate: userDelegate
                        subtitle: userDelegate.connection.localUser.id
                        labelItem.textFormat: Text.PlainText
                        subtitleItem.textFormat: Text.PlainText
                    }
                }

                onClicked: {
                    Controller.activeConnection = userDelegate.connection
                    if (switchUserButton.checked) {
                        switchUserButton.checked = false
                    }
                }
            }
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
}
