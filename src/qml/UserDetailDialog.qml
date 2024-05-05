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

    /**
     * @brief The user's profile object.
     *
     * Required to interact with the profile and perform action like ignoring.
     */
    property var user

    /**
     * @brief The RoomMember object for the user in the current room.
     *
     * Required to visualise the user.
     */
    property var member: root.room.member(user.id)

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

                name: root.member.displayName
                source: root.member.avatarUrl
                color: root.member.color
            }

            ColumnLayout {
                Layout.fillWidth: true

                Kirigami.Heading {
                    level: 1
                    Layout.fillWidth: true
                    font.bold: true

                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                    text: root.user.displayName
                    textFormat: Text.PlainText
                }

                Kirigami.SelectableLabel {
                    textFormat: TextEdit.PlainText
                    text: root.member.id
                }
            }
            QQC2.AbstractButton {
                Layout.minimumHeight: avatar.height * 0.75
                Layout.maximumHeight: avatar.height * 1.5
                contentItem: Barcode {
                    id: barcode
                    barcodeType: Barcode.QRCode
                    content: "https://matrix.to/#/" + root.member.id
                }

                onClicked: {
                    let map = qrMaximizeComponent.createObject(parent, {
                        text: barcode.content,
                        title: root.member.displayName,
                        subtitle: root.member.id,
                        avatarColor: root.member.color,
                        avatarSource: root.member.avatarUrl,
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
            visible: !root.member.isLocalMember
            action: Kirigami.Action {
                text: !!root.user && root.connection.isIgnored(root.user) ? i18n("Unignore this user") : i18n("Ignore this user")
                icon.name: "im-invisible-user"
                onTriggered: {
                    root.close();
                    root.connection.isIgnored(root.user) ? root.connection.removeFromIgnoredUsers(root.user) : root.connection.addToIgnoredUsers(root.user);
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && !root.member.isLocalMember && room.canSendState("kick") && room.containsUser(root.member.id) && room.getUserPowerLevel(root.member.id) < room.getUserPowerLevel(root.room.localMember.id)

            action: Kirigami.Action {
                text: i18n("Kick this user")
                icon.name: "im-kick-user"
                onTriggered: {
                    root.room.kickMember(root.member.id);
                    root.close();
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && !root.member.isLocalMember && room.canSendState("invite") && !room.containsUser(root.member.id)

            action: Kirigami.Action {
                enabled: root.room && !root.room.isUserBanned(root.member.id)
                text: i18n("Invite this user")
                icon.name: "list-add-user"
                onTriggered: {
                    root.room.inviteToRoom(root.member.id);
                    root.close();
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && !root.member.isLocalMember && room.canSendState("ban") && !room.isUserBanned(root.member.id) && room.getUserPowerLevel(root.member.id) < room.getUserPowerLevel(root.room.localMember.id)

            action: Kirigami.Action {
                text: i18n("Ban this user")
                icon.name: "im-ban-user"
                icon.color: Kirigami.Theme.negativeTextColor
                onTriggered: {
                    (root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'BanSheet'), {
                        room: root.room,
                        userId: root.member.id
                    }, {
                        title: i18nc("@title", "Ban User"),
                        width: Kirigami.Units.gridUnit * 25
                    });
                    root.close();
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: root.room && !root.member.isLocalMember && room.canSendState("ban") && room.isUserBanned(root.member.id)

            action: Kirigami.Action {
                text: i18n("Unban this user")
                icon.name: "im-irc"
                icon.color: Kirigami.Theme.negativeTextColor
                onTriggered: {
                    root.room.unban(root.member.id);
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
                        userId: root.member.id,
                        powerLevel: root.room.getUserPowerLevel(root.member.id)
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
            visible: root.room && (root.member.isLocalUser || room.canSendState("redact"))

            action: Kirigami.Action {
                text: i18n("Remove recent messages by this user")
                icon.name: "delete"
                icon.color: Kirigami.Theme.negativeTextColor
                onTriggered: {
                    applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'RemoveSheet'), {
                        room: root.room,
                        userId: root.member.id
                    }, {
                        title: i18nc("@title", "Remove Messages"),
                        width: Kirigami.Units.gridUnit * 25
                    });
                    root.close();
                }
            }
        }

        FormCard.FormButtonDelegate {
            visible: !root.member.isLocalMember
            action: Kirigami.Action {
                text: root.connection.directChatExists(root.user.object) ? i18nc("%1 is the name of the user.", "Chat with %1", root.user.escapedDisplayName) : i18n("Invite to private chat")
                icon.name: "document-send"
                onTriggered: {
                    root.room.connection.openOrCreateDirectChat(root.user);
                    root.close();
                }
            }
        }

        FormCard.FormButtonDelegate {
            action: Kirigami.Action {
                text: i18n("Copy link")
                icon.name: "username-copy"
                onTriggered: {
                    Clipboard.saveText("https://matrix.to/#/" + root.member.id);
                }
            }
        }
    }
    Component {
        id: qrMaximizeComponent
        QrCodeMaximizeComponent {}
    }
}
