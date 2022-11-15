// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0

QQC2.ToolBar {
    id: userInfo

    padding: 0

    height: content.height

    property alias accountsListVisible: accounts.visible
    property var addAccount

    ColumnLayout {
        id: content
        width: parent.width

        spacing: 0

        ListView {
            id: accounts

            currentIndex: Controller.activeConnectionIndex

            header: Kirigami.Separator {}

            footer: Kirigami.BasicListItem {
                width: parent.width
                highlighted: focus
                background: Rectangle {
                    id: background
                    color: addAccount.backgroundColor

                    Rectangle {
                        anchors.fill: parent
                        visible: !Kirigami.Settings.tabletMode && addAccount.hoverEnabled
                        color: addAccount.activeBackgroundColor
                        opacity: {
                            if ((addAccount.highlighted || addAccount.ListView.isCurrentItem) && !addAccount.pressed) {
                                return .6
                            } else if (addAccount.hovered && !addAccount.pressed) {
                                return .3
                            } else {
                                return 0
                            }
                        }
                    }
                }
                Component.onCompleted: userInfo.addAccount = this
                icon: "list-add"
                text: i18n("Add Account")
                subtitle: i18n("Log in to an existing account")
                onClicked: {
                    pageStack.pushDialogLayer("qrc:/WelcomePage.qml", {}, {title: i18nc("@title:window", "Login")})
                    userInfo.accountsListVisible = false
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

            visible: false
            onVisibleChanged: if (visible) focus = true
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
                userInfo.accountsListVisible = false
            }

            width: parent.width
            Layout.preferredHeight: contentHeight
            delegate: Kirigami.BasicListItem {
                leftPadding: topPadding
                leading: Kirigami.Avatar {
                    width: height
                    source: model.connection.localUser.avatarMediaId ? ("image://mxc/" + model.connection.localUser.avatarMediaId) : ""
                    name: model.connection.localUser.displayName ?? model.connection.localUser.id
                }
                width: parent.width
                text: model.connection.localUser.displayName
                labelItem.textFormat: Text.PlainText
                subtitleItem.textFormat: Text.PlainText
                subtitle: model.connection.localUser.id

                onClicked: {
                    Controller.activeConnection = model.connection
                    userInfo.accountsListVisible = false
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.preferredHeight: Kirigami.Units.gridUnit * 3
            Item {
                Layout.fillHeight: true
                Layout.preferredWidth: height
                Kirigami.Avatar {
                    readonly property string mediaId: Controller.activeConnection.localUser.avatarMediaId
                    anchors.fill: parent
                    anchors.margins: Kirigami.Units.smallSpacing
                    source: mediaId ? ("image://mxc/" + mediaId) : ""
                    name: Controller.activeConnection.localUser.displayName ?? Controller.activeConnection.localUser.id
                    actions.main: Kirigami.Action {
                        text: i18n("Edit this account")
                        iconName: "document-edit"
                        onTriggered: pageStack.pushDialogLayer(Qt.resolvedUrl('./AccountEditorPage.qml'), {
                            connection: Controller.activeConnection
                        }, {
                            title: i18n("Account editor")
                        });
                    }
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
                    text: (Controller.activeConnection.localUser.accountLabel.length > 0 ? (Controller.activeConnection.localUser.accountLabel + " ") : "") + Controller.activeConnection.localUser.id
                    font.pointSize: displayNameLabel.font.pointSize * 0.8
                    opacity: 0.7
                    textFormat: Text.PlainText
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
            QQC2.ToolButton {
                icon.name: "system-switch-user"
                onClicked: {
                    userInfo.accountsListVisible = !userInfo.accountsListVisible
                }
                text: i18n("Switch User")
                display: QQC2.AbstractButton.IconOnly
                Accessible.name: text
                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                Layout.minimumWidth: Layout.preferredWidth
                Layout.alignment: Qt.AlignRight
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
                onClicked: pageStack.pushDialogLayer("qrc:/SettingsPage.qml", {}, { title: i18n("Configure") })
                text: i18n("Open Settings")
                display: QQC2.AbstractButton.IconOnly
                Layout.minimumWidth: Layout.preferredWidth
                Layout.alignment: Qt.AlignRight
                QQC2.ToolTip {
                    text: parent.text
                }
            }
            Item {
                width: 1
            }
        }
    }
}
