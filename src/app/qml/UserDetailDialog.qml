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
            id: detailRow
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

                spacing: 0

                Kirigami.Heading {
                    level: 1
                    Layout.fillWidth: true
                    font.bold: true
                    clip: true // Intentional to limit insane Unicode in display names

                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                    text: root.room ? root.room.member(root.user.id).displayName : root.user.displayName
                    textFormat: Text.PlainText
                }

                Kirigami.SelectableLabel {
                    id: idLabel
                    textFormat: TextEdit.PlainText
                    text: idLabelTextMetrics.elidedText

                    TextMetrics {
                        id: idLabelTextMetrics
                        text: root.user.id
                        elide: Qt.ElideRight
                        elideWidth: root.availableWidth - avatar.width - qrButton.width - detailRow.spacing * 2 - detailRow.Layout.leftMargin - detailRow.Layout.rightMargin
                    }
                }

                QQC2.Label {
                    property CommonRoomsModel model: CommonRoomsModel {
                        connection: root.connection
                        userId: root.user.id
                    }

                    text: i18ncp("@info", "One mutual room", "%1 mutual rooms", model.count)
                    color: Kirigami.Theme.disabledTextColor
                    visible: model.count > 0

                    Layout.topMargin: Kirigami.Units.smallSpacing
                }
            }
            QQC2.AbstractButton {
                id: qrButton
                Layout.minimumHeight: avatar.height * 0.75
                Layout.maximumHeight: avatar.height * 1.5
                Layout.maximumWidth: avatar.height * 1.5

                contentItem: Barcode {
                    id: barcode
                    barcodeType: Barcode.QRCode
                    content: "https://matrix.to/#/" + root.user.id
                }

                onClicked: {
                    let qrCode = Qt.createComponent('org.kde.neochat', 'QrCodeMaximizeComponent').createObject(QQC2.Overlay.overlay, {
                        text: barcode.content,
                        title: root.room ? root.room.member(root.user.id).displayName : root.user.displayName,
                        subtitle: root.user.id,
                        avatarColor: root.room?.member(root.user.id).color,
                        avatarSource: root.room? root.room.member(root.user.id).avatarUrl : root.user.avatarUrl
                    }) as QrCodeMaximizeComponent;
                    root.close();
                    qrCode.open();
                }

                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: barcode.content
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }

        Kirigami.Chip {
            visible: root.room
            text: root.room ? QmlUtils.nameForPowerLevelValue(root.room.memberEffectivePowerLevel(root.user.id)) : ""
            closable: false
            checkable: false

            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        FormCard.FormButtonDelegate {
            visible: root.user.id !== root.connection.localUserId && !!root.user
            text: !!root.user && root.connection.isIgnored(root.user.id) ? i18n("Unignore this user") : i18n("Ignore this user")
            icon.name: "im-invisible-user"
            onClicked: {
                root.close();
                root.connection.isIgnored(root.user.id) ? root.connection.removeFromIgnoredUsers(root.user.id) : root.connection.addToIgnoredUsers(root.user.id);
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && root.user.id !== root.connection.localUserId && root.room.canSendState("kick") && root.room.containsUser(root.user.id) && root.room.memberEffectivePowerLevel(root.user.id) < root.room.memberEffectivePowerLevel(root.connection.localUserId)

            text: i18nc("@action:button", "Kick this user")
            icon.name: "im-kick-user"
            onClicked: {
                let dialog = (root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
                    title: i18nc("@title:dialog", "Kick User"),
                    placeholder: i18nc("@info:placeholder", "Reason for kicking this user"),
                    actionText: i18nc("@action:button 'Kick' as in 'Kick this user from the room'", "Kick"),
                    icon: "im-kick-user"
                }, {
                    title: i18nc("@title:dialog", "Kick User"),
                    width: Kirigami.Units.gridUnit * 25
                });
                dialog.accepted.connect(reason => {
                    root.room.kickMember(root.user.id, reason);
                });
                root.close();
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && root.user.id !== root.connection.localUserId && root.room.canSendState("invite") && !root.room.containsUser(root.user.id)

            enabled: root.room && !root.room.isUserBanned(root.user.id)
            text: i18nc("@action:button", "Invite this user")
            icon.name: "list-add-user"
            onClicked: {
                root.room.inviteToRoom(root.user.id);
                root.close();
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && root.user.id !== root.connection.localUserId && root.room.canSendState("ban") && !root.room.isUserBanned(root.user.id) && root.room.memberEffectivePowerLevel(root.user.id) < root.room.memberEffectivePowerLevel(root.connection.localUserId)

            text: i18nc("@action:button", "Ban this user")
            icon.name: "im-ban-user"
            icon.color: Kirigami.Theme.negativeTextColor
            onClicked: {
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

        FormCard.FormButtonDelegate {
            visible: root.room && root.user.id !== root.connection.localUserId && root.room.canSendState("ban") && root.room.isUserBanned(root.user.id)

            text: i18nc("@action:button", "Unban this user")
            icon.name: "im-irc"
            icon.color: Kirigami.Theme.negativeTextColor
            onClicked: {
                root.room.unban(root.user.id);
                root.close();
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && root.room.canSendState("m.room.power_levels")
            text: i18nc("@action:button", "Set user power level")
            icon.name: "visibility"
            onClicked: {
                (powerLevelDialog.createObject(this, {
                    room: root.room,
                    userId: root.user.id,
                    powerLevel: root.room.memberEffectivePowerLevel(root.user.id)
                }) as PowerLevelDialog).open();
                root.close();
            }

            Component {
                id: powerLevelDialog
                PowerLevelDialog {
                    id: powerLevelDialog
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && (root.user.id === root.connection.localUserId || root.room.canSendState("redact"))

            text: i18nc("@action:button", "Remove recent messages by this user")
            icon.name: "delete"
            icon.color: Kirigami.Theme.negativeTextColor
            onClicked: {
                let dialog = ((QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
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

        FormCard.FormButtonDelegate {
            visible: root.user.id !== root.connection.localUserId
            text: root.connection.directChatExists(root.user) ? i18nc("%1 is the name of the user.", "Chat with %1", root.room ? root.room.member(root.user.id).htmlSafeDisplayName : QmlUtils.escapeString(root.user.displayName)) : i18n("Invite to private chat")
            icon.name: "document-send"
            onClicked: {
                root.connection.requestDirectChat(root.user.id);
                root.close();
            }
        }

        FormCard.FormButtonDelegate {
            text: i18nc("@action:button %1 is the name of the user.", "Search room for %1's messages", root.room ? root.room.member(root.user.id).htmlSafeDisplayName : QmlUtils.escapeString(root.user.displayName))
            icon.name: "search-symbolic"
            onClicked: {
                ((QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'RoomSearchPage'), {
                    room: root.room,
                    senderId: root.user.id
                }, {
                    title: i18nc("@action:title", "Search")
                });
                root.close();
            }
        }

        FormCard.FormButtonDelegate {
            text: i18n("Copy link")
            icon.name: "username-copy"
            onClicked: Clipboard.saveText("https://matrix.to/#/" + root.user.id)
        }

        FormCard.FormButtonDelegate {
            text: i18nc("@action:button 'Report' as in 'Report this user to the administrators'", "Report…")
            icon.name: "dialog-warning-symbolic"
            visible: root.connection.supportsMatrixSpecVersion("v1.13")
            onClicked: {
                let dialog = (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
                    title: i18nc("@title:dialog", "Report User"),
                    placeholder: i18nc("@info:placeholder", "Reason for reporting this user"),
                    icon: "dialog-warning-symbolic",
                    actionText: i18nc("@action:button 'Report' as in 'Report this user to the administrators'", "Report")
                }, {
                    title: i18nc("@title", "Report User"),
                    width: Kirigami.Units.gridUnit * 25
                }) as ReasonDialog;
                dialog.accepted.connect(reason => {
                    root.connection.reportUser(root.user.id, reason);
                });
            }
        }
    }
}
