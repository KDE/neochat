/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami
import NeoChat.Component.Timeline 1.0
import org.kde.neochat 1.0

QQC2.AbstractButton {
    visible: replyVisible
    Component.onCompleted: parent.Layout.fillWidth = true

    contentItem: RowLayout {
        Layout.fillWidth: true

        Rectangle {
            Layout.preferredWidth: Kirigami.Units.smallSpacing
            Layout.fillHeight: true

            color: Kirigami.Theme.highlightColor
        }

        Kirigami.Avatar {
            Layout.preferredWidth: Kirigami.Units.gridUnit
            Layout.preferredHeight: Kirigami.Units.gridUnit
            Layout.alignment: Qt.AlignTop
            visible: Config.showAvatarInTimeline
            source: replyVisible && reply.author.avatarMediaId ? ("image://mxc/" + reply.author.avatarMediaId) : ""
            name: replyVisible ? (reply.author.name || "") : "H"
            color: replyVisible ? reply.author.color : Kirigami.Theme.highlightColor
        }

        ColumnLayout {
            id: replyLayout
            Layout.fillWidth: true

            QQC2.Label {
                Layout.fillWidth: true
                text: replyVisible ? reply.author.displayName : ""
                color: replyVisible ? reply.author.color: null
                elide: Text.ElideRight
            }

            TextDelegate {
                Layout.fillWidth: true
                Layout.leftMargin: 0
                text: replyVisible ? ("<style>pre {white-space: pre-wrap} a{color: " + Kirigami.Theme.linkColor + ";} .user-pill{}</style>" + reply.display) : ""
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
            }
        }
    }
}
