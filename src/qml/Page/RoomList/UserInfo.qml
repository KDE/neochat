// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents
import org.kde.kirigamiaddons.delegates 1.0 as Delegates

import org.kde.neochat 1.0

QQC2.ToolBar {
    id: userInfo

    padding: 0

    property var addAccount

    contentItem: ColumnLayout {
        id: content

        spacing: 0

        ListView {
            id: accounts

            currentIndex: Controller.activeConnectionIndex

            header: Kirigami.Separator {}

            footer: Delegates.RoundedItemDelegate {
                id: addButton
                width: parent.width
                highlighted: focus || (addAccount.highlighted || addAccount.ListView.isCurrentItem) && !addAccount.pressed
                Component.onCompleted: userInfo.addAccount = this
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
                    pageStack.pushDialogLayer("qrc:/WelcomePage.qml", {}, {
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

            visible: switchUserButton.checked
            onVisibleChanged: if (visible) accounts.forceActiveFocus()
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

            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            delegate: Delegates.RoundedItemDelegate {
                id: userDelegate

                required property var connection

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

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            Layout.minimumHeight: Kirigami.Units.gridUnit * 2 - 2 // HACK: -2 here is to ensure the ChatBox and the UserInfo have the same height

            QQC2.AbstractButton {
                id: accountButton

                Layout.preferredWidth: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
                Layout.preferredHeight: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
                Layout.leftMargin: Kirigami.Units.largeSpacing

                TapHandler {
                    acceptedButtons: Qt.RightButton | Qt.LeftButton
                   acceptedDevices: PointerDevice.Mouse
                    onTapped: (eventPoint, button) => {
                        // TODO Qt6 remove
                        if (!button) {
                            button = eventPoint.event.button;
                        }
                        if (button == Qt.RightButton) {
                            accountMenu.open();
                        } else {
                            pageStack.pushDialogLayer(Qt.resolvedUrl('qrc:/AccountEditorPage.qml'), {
                                connection: Controller.activeConnection
                            }, {
                                title: i18n("Account editor")
                            });
                        }
                    }
                }

                text: i18n("Edit this account")

                contentItem: KirigamiComponents.Avatar {
                    readonly property string mediaId: Controller.activeConnection.localUser.avatarMediaId

                    source: mediaId ? ("image://mxc/" + mediaId) : ""
                    name: Controller.activeConnection.localUser.displayName ?? Controller.activeConnection.localUser.id
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0
                QQC2.Label {
                    id: displayNameLabel
                    text: Controller.activeConnection.localUser.displayName
                    textFormat: Text.PlainText
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: (Controller.activeAccountLabel.length > 0 ? (Controller.activeAccountLabel + " ") : "") + Controller.activeConnection.localUser.id
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
                onClicked: pageStack.pushDialogLayer("qrc:/SettingsPage.qml", {connection: Controller.activeConnection}, { title: i18n("Configure") })
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
                y: -height
            }
        }
    }
}
