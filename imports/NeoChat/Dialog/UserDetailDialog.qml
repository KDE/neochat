// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0

Kirigami.OverlaySheet {
    id: root

    property var room
    property var user

    property string displayName: user.displayName
    property string avatarMediaId: user.avatarMediaId
    property string avatarUrl: user.avatarUrl

    parent: applicationWindow().overlay

    leftPadding: 0
    rightPadding: 0
    topPadding: 0

    header: Kirigami.Heading {
        id: heading
        text: i18nc("@title:menu Account detail dialog", "Account detail")
        elide: Text.ElideRight
        QQC2.ToolTip.visible: truncated && hovered
        QQC2.ToolTip.text: text
    }

    contentItem: ColumnLayout {
        spacing: 0
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Avatar {
                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge

                name: displayName
                source: avatarMediaId ? ("image://mxc/" + avatarMediaId) : ""
                color: user.color

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        if (avatarMediaId) {
                            fullScreenImage.createObject(parent, {"filename": displayName, "localPath": room.urlToMxcUrl(avatarUrl)}).showFullScreen()
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true

                Kirigami.Heading {
                    level: 1
                    Layout.fillWidth: true
                    font.bold: true

                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                    text: displayName
                }

                QQC2.Label {
                    Layout.fillWidth: true

                    text: i18n("Online")
                    color: Kirigami.Theme.disabledTextColor
                }
                Kirigami.Heading {
                    level: 5
                    text: user.id
                }
            }
        }

        QQC2.MenuSeparator {}

        Kirigami.BasicListItem {
            visible: user !== room.localUser
            action: Kirigami.Action {
                text: room.connection.isIgnored(user) ? i18n("Unignore this user") : i18n("Ignore this user")
                icon.name: "im-invisible-user"
                onTriggered: {
                    root.close()
                    room.connection.isIgnored(user) ? room.connection.removeFromIgnoredUsers(user) : room.connection.addToIgnoredUsers(user)
                }
            }
        }
        Kirigami.BasicListItem {
            visible: user !== room.localUser && room.canSendState("kick") && room.containsUser(user)

            action: Kirigami.Action {
                text: i18n("Kick this user")
                icon.name: "im-kick-user"
                onTriggered: {
                    room.kickMember(user.id)
                    root.close()
                }
            }
        }
        Kirigami.BasicListItem {
            visible: user !== room.localUser && room.canSendState("ban")

            action: Kirigami.Action {
                text: i18n("Ban this user")
                icon.name: "im-ban-user"
                icon.color: Kirigami.Theme.negativeTextColor
                onTriggered: {
                    room.banMember(user.id)
                    root.close()
                }
            }
        }
        Kirigami.BasicListItem {
            action: Kirigami.Action {
                text: i18n("Open a private chat")
                icon.name: "document-send"
                onTriggered: {
                    Controller.activeConnection.requestDirectChat(user)
                    root.close()
                }
            }
        }
        Component {
            id: fullScreenImage

            FullScreenImage {}
        }
    }
}

