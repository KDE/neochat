// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.prison

import org.kde.neochat

Kirigami.Dialog {
    id: root

    property NeoChatRoom room
    property var user

    parent: applicationWindow().overlay

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    standardButtons: Kirigami.Dialog.NoButton

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    title: i18nc("@title:menu Account details dialog", "Account Details")

    contentItem: ColumnLayout {
        spacing: 0
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            spacing: Kirigami.Units.largeSpacing

            KirigamiComponents.Avatar {
                id: avatar
                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge

                name: root.user.displayName
                source: root.user.avatarSource
                color: root.user.color
            }

            ColumnLayout {
                Layout.fillWidth: true

                Kirigami.Heading {
                    level: 1
                    Layout.fillWidth: true
                    font.bold: true

                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                    text: user.displayName
                    textFormat: Text.PlainText
                }

                Kirigami.SelectableLabel {
                    textFormat: TextEdit.PlainText
                    text: root.user.id
                }
            }
            QQC2.AbstractButton {
                Layout.minimumHeight: avatar.height * 0.75
                Layout.maximumHeight: avatar.height * 1.5
                contentItem: Barcode {
                    id: barcode
                    barcodeType: Barcode.QRCode
                    content: "https://matrix.to/#/" + root.user.id
                }

                onClicked: {
                    let map = qrMaximizeComponent.createObject(parent, {
                        text: barcode.content,
                        title: root.user.displayName,
                        subtitle: root.user.id,
                        avatarColor: root.user.color,
                        avatarSource: root.user.avatarSource,
                    });
                    root.close()
                    map.open()
                }

                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: barcode.content
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        FormCard.FormButtonDelegate {
            visible: !root.user.isLocalUser
            action: Kirigami.Action {
                text: room.connection.isIgnored(root.user.object) ? i18n("Unignore this user") : i18n("Ignore this user")
                icon.name: "im-invisible-user"
                onTriggered: {
                    root.close()
                    room.connection.isIgnored(root.user.object) ? room.connection.removeFromIgnoredUsers(root.user.object) : room.connection.addToIgnoredUsers(root.user.object)
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: !root.user.isLocalUser && room.canSendState("kick") && room.containsUser(root.user.id) && room.getUserPowerLevel(root.user.id) < room.getUserPowerLevel(root.room.connection.localUser.id)

            action: Kirigami.Action {
                text: i18n("Kick this user")
                icon.name: "im-kick-user"
                onTriggered: {
                    room.kickMember(root.user.id)
                    root.close()
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: !root.user.isLocalUser && room.canSendState("invite") && !room.containsUser(root.user.id)

            action: Kirigami.Action {
                enabled: !room.isUserBanned(root.user.id)
                text: i18n("Invite this user")
                icon.name: "list-add-user"
                onTriggered: {
                    room.inviteToRoom(root.user.id)
                    root.close()
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: !root.user.isLocalUser && room.canSendState("ban") && !room.isUserBanned(root.user.id) && room.getUserPowerLevel(root.user.id) < room.getUserPowerLevel(root.room.connection.localUser.id)

            action: Kirigami.Action {
                text: i18n("Ban this user")
                icon.name: "im-ban-user"
                icon.color: Kirigami.Theme.negativeTextColor
                onTriggered: {
                    applicationWindow().pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/BanSheet.qml", {room: root.room, userId: root.user.id}, {
                        title: i18nc("@title", "Ban User"),
                        width: Kirigami.Units.gridUnit * 25
                    })
                    root.close()
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: !root.user.isLocalUser && room.canSendState("ban") && room.isUserBanned(root.user.id)

            action: Kirigami.Action {
                text: i18n("Unban this user")
                icon.name: "im-irc"
                icon.color: Kirigami.Theme.negativeTextColor
                onTriggered: {
                    room.unban(root.user.id)
                    root.close()
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: room.canSendState("m.room.power_levels")
            action: Kirigami.Action {
                text: i18n("Set user power level")
                icon.name: "visibility"
                onTriggered: {
                    let dialog = powerLevelDialog.createObject(applicationWindow().overlay, {
                        room: root.room,
                        userId: root.user.id,
                        powerLevel: root.room.getUserPowerLevel(root.user.id)
                    });
                    dialog.open()
                    root.close()
                }
            }

            Component {
                id: powerLevelDialog
                PowerLevelDialog {
                    id: powerLevelDialog
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.user.isLocalUser || room.canSendState("redact")

            action: Kirigami.Action {
                text: i18n("Remove recent messages by this user")
                icon.name: "delete"
                icon.color: Kirigami.Theme.negativeTextColor
                onTriggered: {
                    applicationWindow().pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/RemoveSheet.qml", {room: root.room, userId: root.user.id}, {
                        title: i18nc("@title", "Remove Messages"),
                        width: Kirigami.Units.gridUnit * 25
                    })
                    root.close()
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: !root.user.isLocalUser
            action: Kirigami.Action {
                text: root.room.connection.directChatExists(root.user.object) ? i18nc("%1 is the name of the user.", "Chat with %1", root.user.displayName) : i18n("Invite to private chat")
                icon.name: "document-send"
                onTriggered: {
                    root.room.connection.openOrCreateDirectChat(root.user.object)
                    root.close()
                }
            }
        }

        FormCard.FormButtonDelegate {
            action: Kirigami.Action {
                text: i18n("Copy link")
                icon.name: "username-copy"
                onTriggered: {
                    Clipboard.saveText("https://matrix.to/#/" + root.user.id)
                }
            }
        }
    }
    Component {
        id: qrMaximizeComponent
        QrCodeMaximizeComponent {}
    }
}

