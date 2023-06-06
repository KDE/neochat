// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0

Kirigami.OverlaySheet {
    id: root

    signal closed()

    property NeoChatRoom room
    property var user

    parent: applicationWindow().overlay

    leftPadding: Kirigami.Units.smallSpacing
    rightPadding: Kirigami.Units.smallSpacing
    topPadding: 0
    bottomPadding: 0

    title: i18nc("@title:menu Account detail dialog", "Account detail")

    contentItem: ColumnLayout {
        spacing: 0
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Avatar {
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
                }

                Kirigami.SelectableLabel {
                    textFormat: TextEdit.PlainText
                    text: root.user.id
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        Kirigami.BasicListItem {
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
        Kirigami.BasicListItem {
            visible: !root.user.isLocalUser && room.canSendState("kick") && room.containsUser(root.user.id)

            action: Kirigami.Action {
                text: i18n("Kick this user")
                icon.name: "im-kick-user"
                onTriggered: {
                    room.kickMember(root.user.id)
                    root.close()
                }
            }
        }
        Kirigami.BasicListItem {
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
        Kirigami.BasicListItem {
            visible: !root.user.isLocalUser && room.canSendState("ban") && !room.isUserBanned(root.user.id)

            action: Kirigami.Action {
                text: i18n("Ban this user")
                icon.name: "im-ban-user"
                icon.color: Kirigami.Theme.negativeTextColor
                onTriggered: {
                    applicationWindow().pageStack.pushDialogLayer("qrc:/BanSheet.qml", {room: root.room, userId: root.user.id}, {
                        title: i18nc("@title", "Ban User"),
                        width: Kirigami.Units.gridUnit * 25
                    })
                    root.close()
                }
            }
        }
        Kirigami.BasicListItem {
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
        Kirigami.BasicListItem {
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
        Kirigami.BasicListItem {
            visible: root.user.isLocalUser || room.canSendState("redact")

            action: Kirigami.Action {
                text: i18n("Remove recent messages by this user")
                icon.name: "delete"
                icon.color: Kirigami.Theme.negativeTextColor
                onTriggered: {
                    applicationWindow().pageStack.pushDialogLayer("qrc:/RemoveSheet.qml", {room: root.room, userId: root.user.id}, {
                        title: i18nc("@title", "Remove Messages"),
                        width: Kirigami.Units.gridUnit * 25
                    })
                    root.close()
                }
            }
        }
        Kirigami.BasicListItem {
            visible: !root.user.isLocalUser
            action: Kirigami.Action {
                text: i18n("Open a private chat")
                icon.name: "document-send"
                onTriggered: {
                    Controller.openOrCreateDirectChat(root.user.object);
                    root.close()
                }
            }
        }
        Kirigami.BasicListItem {
            action: Kirigami.Action {
                text: i18n("Copy link")
                icon.name: "username-copy"
                onTriggered: {
                    Clipboard.saveText("https://matrix.to/#/" + root.user.id)
                }
            }
        }
    }

    onSheetOpenChanged: {
        if (!sheetOpen) {
            closed()
        }
    }
}

