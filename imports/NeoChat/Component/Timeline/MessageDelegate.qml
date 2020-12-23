/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12

import org.kde.kirigami 2.13 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Setting 1.0
import NeoChat.Component 1.0
import NeoChat.Dialog 1.0

RowLayout {
    default property alias innerObject : column.children

    readonly property bool sentByMe: author.isLocalUser
    readonly property bool darkBackground: !sentByMe
    readonly property bool replyVisible: reply ?? false
    readonly property bool failed: marks == EventStatus.SendingFailed
    readonly property color authorColor: eventType == "notice" ? Kirigami.Theme.activeTextColor : author.color
    readonly property color replyAuthorColor: replyVisible ? reply.author.color : Kirigami.Theme.focusColor

    property alias mouseArea: controlContainer.children
    property bool isEmote: false

    signal saveFileAs()
    signal openExternally()
    signal replyClicked(string eventID)
    signal replyToMessageClicked(var replyUser, string replyContent, string eventID)

    id: root

    spacing: Kirigami.Units.smallSpacing
    Layout.leftMargin: Kirigami.Units.largeSpacing
    Layout.rightMargin: Kirigami.Units.smallSpacing
    Layout.bottomMargin: 0
    Layout.topMargin: showAuthor ? Kirigami.Units.smallSpacing : 0

    Kirigami.Avatar {
        Layout.minimumWidth: Kirigami.Units.gridUnit * 2
        Layout.minimumHeight: Kirigami.Units.gridUnit * 2
        Layout.maximumWidth: Kirigami.Units.gridUnit * 2
        Layout.maximumHeight: Kirigami.Units.gridUnit * 2

        Layout.alignment: Qt.AlignTop

        visible: showAuthor && Config.showAvatarInTimeline
        name: author.displayName
        source: author.avatarMediaId ? "image://mxc/" + author.avatarMediaId : ""
        color: author.color

        Component {
            id: userDetailDialog

            UserDetailDialog {}
        }

        MouseArea {
            anchors.fill: parent
            onClicked: userDetailDialog.createObject(QQC2.ApplicationWindow.overlay, {"room": currentRoom, "user": author.object, "displayName": author.displayName, "avatarMediaId": author.avatarMediaId, "avatarUrl": author.avatarUrl}).open()
            cursorShape: Qt.PointingHandCursor
        }
    }

    Item {
        Layout.minimumWidth: Kirigami.Units.gridUnit * 2
        Layout.preferredHeight: 1
        visible: !showAuthor && Config.showAvatarInTimeline
    }


    QQC2.Control {
        id: controlContainer
        Layout.fillWidth: true
        topPadding: 0
        bottomPadding: 0
        hoverEnabled: true
        contentItem: ColumnLayout {
            id: column
            spacing: showAuthor ? Kirigami.Units.smallSpacing : 0

            RowLayout {
                id: rowLayout
                Layout.fillWidth: true
                QQC2.Label {
                    Layout.fillWidth: true
                    topInset: 0

                    visible: showAuthor && !isEmote

                    text: author.displayName
                    font.bold: true
                    color: author.color
                    wrapMode: Text.Wrap
                }
                QQC2.Label {
                    visible: showAuthor && !isEmote
                    text: time.toLocaleTimeString(Locale.ShortFormat)
                    color: Kirigami.Theme.disabledTextColor
                }
            }
            Loader {
                id: replyLoader
                source: 'qrc:imports/NeoChat/Component/Timeline/ReplyComponent.qml'
                active: replyVisible
            }
            Connections {
                target: replyLoader.item
                onClicked: replyClicked(reply.eventId)
            }
        }
        RowLayout {
            z: 2
            anchors.bottom: controlContainer.top
            anchors.bottomMargin: -Kirigami.Units.gridUnit
            anchors.right: controlContainer.right
            spacing: 0
            QQC2.Button {
                QQC2.ToolTip.text: i18n("React")
                QQC2.ToolTip.visible: hovered
                visible: controlContainer.hovered
                icon.name: "preferences-desktop-emoticons"
                onClicked: emojiDialog.open();
                EmojiDialog {
                    id: emojiDialog
                    onReact: currentRoom.toggleReaction(eventId, emoji)
                }
            }
            QQC2.Button {
                QQC2.ToolTip.text: i18n("Reply")
                QQC2.ToolTip.visible: hovered
                visible: controlContainer.hovered
                icon.name: "mail-replied-symbolic"
                onClicked: replyToMessage(author, message, eventId)
            }
        }
        background: Rectangle {
            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            color: !model.isHighlighted ? Kirigami.Theme.backgroundColor : Kirigami.Theme.positiveBackgroundColor
            opacity: controlContainer.hovered || model.isHighlighted ? 1 : 0
        }
    }
}
