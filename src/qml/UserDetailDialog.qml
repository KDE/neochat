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

    // This dialog is sometimes used outside the context of a room, e.g., when scanning a user's QR code.
    // Make sure that code is prepared to deal with this property being null
    property NeoChatRoom room
    property var user

    property NeoChatConnection connection

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    standardButtons: Kirigami.Dialog.NoButton

    width: Math.min(QQC2.ApplicationWindow.window.width, Kirigami.Units.gridUnit * 24)
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

                name: root.room ? root.room.member(root.user.id).displayName : root.user.displayName
                source: root.room ? root.room.member(root.user.id).avatarUrl : root.user.avatarUrl
                color: root.room ? root.room.member(root.user.id).color : QmlUtils.getUserColor(root.user.hueF)
            }

            ColumnLayout {
                Layout.fillWidth: true

                Kirigami.Heading {
                    level: 1
                    Layout.fillWidth: true
                    font.bold: true

                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                    text: root.room ? root.room.member(root.user.id).displayName : root.user.displayName
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
                        title: root.room ? root.room.member(root.user.id).displayName : root.user.displayName,
                        subtitle: root.user.id,
                        avatarColor: root.room?.member(root.user.id).color,
                        avatarSource: root.room? root.room.member(root.user.id).avatarUrl : root.user.avatarUrl
                    });
                    root.close();
                    map.open();
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
            visible: root.user.id !== root.connection.localUserId && !!root.user
            action: Kirigami.Action {
                text: !!root.user && root.connection.isIgnored(root.user.id) ? i18n("Unignore this user") : i18n("Ignore this user")
                icon.name: "im-invisible-user"
                onTriggered: {
                    root.close();
                    root.connection.isIgnored(root.user.id) ? root.connection.removeFromIgnoredUsers(root.user.id) : root.connection.addToIgnoredUsers(root.user.id);
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && root.user.id !== root.connection.localUserId && room.canSendState("kick") && room.containsUser(root.user.id) && room.getUserPowerLevel(root.user.id) < room.getUserPowerLevel(root.connection.localUserId)

            action: Kirigami.Action {
                text: i18n("Kick this user")
                icon.name: "im-kick-user"
                onTriggered: {
                    root.room.kickMember(root.user.id);
                    root.close();
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && root.user.id !== root.connection.localUserId && room.canSendState("invite") && !room.containsUser(root.user.id)

            action: Kirigami.Action {
                enabled: root.room && !root.room.isUserBanned(root.user.id)
                text: i18n("Invite this user")
                icon.name: "list-add-user"
                onTriggered: {
                    root.room.inviteToRoom(root.user.id);
                    root.close();
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && root.user.id !== root.connection.localUserId && room.canSendState("ban") && !room.isUserBanned(root.user.id) && room.getUserPowerLevel(root.user.id) < room.getUserPowerLevel(root.connection.localUserId)

            action: Kirigami.Action {
                text: i18n("Ban this user")
                icon.name: "im-ban-user"
                icon.color: Kirigami.Theme.negativeTextColor
                onTriggered: {
                    let dialog = (root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
                        title: i18nc("@title:dialog", "Ban User"),
                        placeholder: i18nc("@info:placeholder", "Reason for banning this user"),
                        actionText: i18nc("@action:button 'Ban' as in 'Ban this user'", "Ban"),
                        icon: "im-ban-user"
                    }, {
                        title: i18nc("@title:dialog", "Ban User"),
                        width: Kirigami.Units.gridUnit * 25
                    });
                    dialog.accepted.connect(reason => {
                        root.room.ban(root.user.id, reason);
                    });
                    root.close();
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && root.user.id !== root.connection.localUserId && room.canSendState("ban") && room.isUserBanned(root.user.id)

            action: Kirigami.Action {
                text: i18n("Unban this user")
                icon.name: "im-irc"
                icon.color: Kirigami.Theme.negativeTextColor
                onTriggered: {
                    root.room.unban(root.user.id);
                    root.close();
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && root.room.canSendState("m.room.power_levels")
            action: Kirigami.Action {
                text: i18n("Set user power level")
                icon.name: "visibility"
                onTriggered: {
                    let dialog = powerLevelDialog.createObject(this, {
                        room: root.room,
                        userId: root.user.id,
                        powerLevel: root.room.getUserPowerLevel(root.user.id)
                    });
                    dialog.open();
                    root.close();
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
            visible: root.room && (root.user.id === root.connection.localUserId || room.canSendState("redact"))

            action: Kirigami.Action {
                text: i18nc("@action:button", "Remove recent messages by this user")
                icon.name: "delete"
                icon.color: Kirigami.Theme.negativeTextColor
                onTriggered: {
                    let dialog = applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
                        title: i18nc("@title:dialog", "Remove Messages"),
                        placeholder: i18nc("@info:placeholder", "Reason for removing this user's recent messages"),
                        actionText: i18nc("@action:button 'Remove' as in 'Remove these messages'", "Remove"),
                        icon: "delete"
                    }, {
                        title: i18nc("@title", "Remove Messages"),
                        width: Kirigami.Units.gridUnit * 25
                    });
                    dialog.accepted.connect(reason => {
                        root.room.deleteMessagesByUser(root.user.id, reason);
                    });
                    root.close();
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.user.id !== root.connection.localUserId
            action: Kirigami.Action {
                text: root.connection.directChatExists(root.user) ? i18nc("%1 is the name of the user.", "Chat with %1", root.room ? root.room.member(root.user.id).htmlSafeDisplayName : QmlUtils.escapeString(root.user.displayName)) : i18n("Invite to private chat")
                icon.name: "document-send"
                onTriggered: {
                    root.connection.openOrCreateDirectChat(root.user.id);
                    root.close();
                }
            }
        }

        FormCard.FormButtonDelegate {
            action: Kirigami.Action {
                text: i18n("Copy link")
                icon.name: "username-copy"
                onTriggered: {
                    Clipboard.saveText("https://matrix.to/#/" + root.user.id);
                }
            }
        }
    }
    Component {
        id: qrMaximizeComponent
        QrCodeMaximizeComponent {}
    }
}
