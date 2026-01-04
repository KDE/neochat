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
    // Used for the "View Main Profile" feature so you can toggle back to the room profile.
    property NeoChatRoom oldRoom

    property NeoChatConnection connection

    property CommonRoomsModel model: CommonRoomsModel {
        connection: root.connection
        userId: root.user.id
    }

    property LimiterModel limiterModel: LimiterModel {
        maximumCount: 5
        sourceModel: root.model
    }

    readonly property bool isSelf: root.user.id === root.connection.localUserId
    readonly property bool hasMutualRooms: root.model.count > 0
    readonly property bool isRoomProfile: root.room
    readonly property string shareUrl: "https://matrix.to/#/" + root.user.id

    leftPadding: Kirigami.Units.largeSpacing * 2
    rightPadding: Kirigami.Units.largeSpacing * 2
    topPadding: Kirigami.Units.largeSpacing * 2
    bottomPadding: Kirigami.Units.largeSpacing * 2

    standardButtons: Kirigami.Dialog.NoButton

    width: Math.min(QQC2.ApplicationWindow.window.width, Kirigami.Units.gridUnit * 24)
    title: i18nc("@title:menu Account details dialog", "Account Details")

    header: null

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            id: detailRow

            spacing: Kirigami.Units.largeSpacing

            Layout.fillWidth: true

            KirigamiComponents.Avatar {
                id: avatar
                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge

                name: root.room ? root.room.member(root.user.id).displayName : root.user.displayName
                source: root.room ? root.room.member(root.user.id).avatarUrl : root.connection.makeMediaUrl(root.user.avatarUrl)
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
                    color: Kirigami.Theme.disabledTextColor

                    TextMetrics {
                        id: idLabelTextMetrics
                        text: root.user.id
                        elide: Qt.ElideRight
                        elideWidth: root.availableWidth - avatar.width - detailRow.spacing * 2 - detailRow.Layout.leftMargin - detailRow.Layout.rightMargin
                    }
                }

                Kirigami.ActionToolBar {
                    Layout.topMargin: Kirigami.Units.smallSpacing

                    actions: [
                        Kirigami.Action {
                            text: i18nc("@action:intoolbar Message this user directly", "Message")
                            icon.name: "document-send-symbolic"

                            onTriggered: {
                                root.close();
                                root.connection.requestDirectChat(root.user.id);
                            }
                        },
                        Kirigami.Action {
                            icon.name: "im-invisible-user-symbolic"
                            text: root.connection.isIgnored(root.user.id) ? i18nc("@action:intoolbar Unignore or 'unblock' this user", "Unignore") : i18nc("@action:intoolbar Ignore or 'block' this user", "Ignore")

                            onTriggered: {
                                root.close();
                                root.connection.isIgnored(root.user.id) ? root.connection.removeFromIgnoredUsers(root.user.id) : root.connection.addToIgnoredUsers(root.user.id);
                            }
                        },
                        Kirigami.Action {
                            text: i18nc("@action:intoolbar Copy shareable link for this user", "Copy Link")
                            icon.name: "username-copy-symbolic"

                            onTriggered: Clipboard.saveText(root.shareUrl)
                        },
                        Kirigami.Action {
                            text: i18nc("@action:intoolbar Search for this user's messages.", "Search Messages…")
                            icon.name: "search-symbolic"

                            onTriggered: {
                                ((root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'RoomSearchPage'), {
                                room: root.room,
                                senderId: root.user.id
                            }, {
                                title: i18nc("@action:title", "Search")
                            });
                                root.close();
                            }
                        },
                        Kirigami.Action {
                            text: i18nc("@action:intoolbar", "Show QR Code")
                            icon.name: "view-barcode-qr-symbolic"

                            onTriggered: {
                                let qrCode = Qt.createComponent('org.kde.neochat', 'QrCodeMaximizeComponent').createObject(QQC2.Overlay.overlay, {
                                    text: root.shareUrl,
                                    title: root.room ? root.room.member(root.user.id).displayName : root.user.displayName,
                                    subtitle: root.user.id,
                                    avatarColor: root.room?.member(root.user.id).color,
                                    avatarSource: root.room? root.room.member(root.user.id).avatarUrl : root.user.avatarUrl
                                }) as QrCodeMaximizeComponent;
                                root.close();
                                qrCode.open();
                            }
                        },
                        Kirigami.Action {
                            text: i18nc("@action:button 'Report' as in 'Report this user to the administrators'", "Report…")
                            icon.name: "dialog-warning-symbolic"
                            visible: root.connection.supportsMatrixSpecVersion("v1.13")

                            onTriggered: {
                                let dialog = ((root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
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
                        },
                        Kirigami.Action {
                            visible: root.room

                            text: i18nc("@action:button", "View Main Profile")
                            icon.name: "user-properties-symbolic"
                            onTriggered: {
                                root.oldRoom = root.room;
                                root.room = null;
                            }
                        },
                        Kirigami.Action {
                            visible: !root.room && root.oldRoom

                            text: i18nc("@action:button", "View Room Profile")
                            icon.name: "user-properties-symbolic"
                            onTriggered: {
                                root.room = root.oldRoom;
                                root.oldRoom = null;
                            }
                        }
                    ]
                }
            }
        }

        Kirigami.Heading {
            text: i18nc("@title Moderation actions for this user", "Moderation")
            level: 2
            visible: root.isRoomProfile && moderationToolbar.actions.filter(function (it) { return it.visible; }).length > 0

            Layout.topMargin: Kirigami.Units.largeSpacing
        }

        Kirigami.ActionToolBar {
            id: moderationToolbar

            flat: false
            visible: root.isRoomProfile

            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.smallSpacing

            actions: [
                Kirigami.Action {
                    visible: !root.isSelf && root.room.canSendState("kick") && root.room.containsUser(root.user.id) && root.room.memberEffectivePowerLevel(root.user.id) < root.room.memberEffectivePowerLevel(root.connection.localUserId)

                    text: i18nc("@action:button Kick the user from the room", "Kick…")
                    icon.name: "im-kick-user"
                    onTriggered: {
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
                },
                Kirigami.Action {
                    visible: !root.isSelf && root.room.canSendState("ban") && !root.room.isUserBanned(root.user.id) && root.room.memberEffectivePowerLevel(root.user.id) < root.room.memberEffectivePowerLevel(root.connection.localUserId)

                    text: i18nc("@action:button Ban this user from the room", "Ban…")
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
                },
                Kirigami.Action {
                    visible: !root.isSelf && root.room.canSendState("ban") && root.room.isUserBanned(root.user.id)

                    text: i18nc("@action:button Unban the user from this room", "Unban")
                    icon.name: "im-irc"
                    icon.color: Kirigami.Theme.negativeTextColor
                    onTriggered: {
                        root.room.unban(root.user.id);
                        root.close();
                    }
                },
                Kirigami.Action {
                    visible: (root.user.id === root.connection.localUserId || root.room.canSendState("redact"))

                    text: i18nc("@action:button Remove messages from the user in this room", "Remove Messages…")
                    icon.name: "delete"
                    icon.color: Kirigami.Theme.negativeTextColor
                    onTriggered: {
                        let dialog = ((root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
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
            ]
        }

        Kirigami.Heading {
            text: i18nc("@title Role such as 'Admin' or 'Moderator' for this user", "Role")
            level: 2
            visible: root.isRoomProfile

            Layout.topMargin: Kirigami.Units.largeSpacing
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            visible: root.isRoomProfile

            Layout.topMargin: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: root.room ? QmlUtils.nameForPowerLevelValue(root.room.memberEffectivePowerLevel(root.user.id)) : ""
            }

            QQC2.Button {
                visible: root.room.canSendState("m.room.power_levels") && !(root.room.roomCreatorHasUltimatePowerLevel() && root.room.isCreator(root.user.id))
                text: i18nc("@action:button Set the power level (such as 'Admin') for this user", "Set Power Level")
                icon.name: "document-edit-symbolic"
                display: QQC2.AbstractButton.IconOnly

                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: text
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

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
        }

        Kirigami.Heading {
            text: i18nc("@title The set of common rooms between your current user and the one shown", "Mutual Rooms")
            level: 4
            visible: !root.isSelf && root.hasMutualRooms

            Layout.topMargin: Kirigami.Units.largeSpacing
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            visible: !root.isSelf && root.hasMutualRooms

            Layout.topMargin: Kirigami.Units.smallSpacing

            Repeater {
                model: root.limiterModel

                delegate: KirigamiComponents.AvatarButton {
                    required property string roomName
                    required property string roomAvatar
                    required property string roomId

                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium

                    name: roomName
                    source: roomAvatar

                    onClicked: {
                        root.close();
                        RoomManager.resolveResource(roomId);
                    }

                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: name
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }
            }

            QQC2.Label {
                text: i18ncp("@info:label And '%1' more rooms you have in common with this user, but are not shown", "and 1 more…", "and %1 more…", root.limiterModel.extraCount)
                visible: root.limiterModel.extraCount > 0
                color: Kirigami.Theme.disabledTextColor
            }
        }
    }
}
