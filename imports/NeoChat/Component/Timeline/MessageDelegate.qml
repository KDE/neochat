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

    signal saveFileAs()
    signal openExternally()
    signal replyClicked(string eventID)

    id: root

    spacing: Kirigami.Units.smallSpacing
    Layout.leftMargin: Kirigami.Units.largeSpacing
    Layout.rightMargin: Kirigami.Units.smallSpacing
    Layout.topMargin: showAuthor ? Kirigami.Units.smallSpacing : 0

    Kirigami.Avatar {
        Layout.minimumWidth: Kirigami.Units.gridUnit * 2
        Layout.minimumHeight: Kirigami.Units.gridUnit * 2
        Layout.maximumWidth: Kirigami.Units.gridUnit * 2
        Layout.maximumHeight: Kirigami.Units.gridUnit * 2

        Layout.alignment: Qt.AlignTop

        visible: showAuthor
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
        }
    }

    Item {
        Layout.minimumWidth: Kirigami.Units.gridUnit * 2
        Layout.preferredHeight: 1
        visible: !showAuthor
    }


    QQC2.Control {
        id: controlContainer
        Layout.fillWidth: true
        topPadding: 0
        contentItem: ColumnLayout {
            id: column
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                Layout.fillWidth: true
                topInset: 0

                visible: showAuthor

                text: author.displayName
                font.bold: true
                color: author.color
                wrapMode: Text.Wrap
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
    }
}
