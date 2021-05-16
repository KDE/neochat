// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component.Timeline 1.0

MouseArea {
    id: replyButton
    Layout.fillWidth: true
    implicitHeight: replyName.implicitHeight + replyText.implicitHeight + Kirigami.Units.largeSpacing
    implicitWidth: Math.min(bubbleMaxWidth, Math.max(replyText.implicitWidth, replyName.implicitWidth)) + Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 3
    Component.onCompleted: {
        parent.Layout.fillWidth = true;
        parent.Layout.preferredWidth = Qt.binding(function() { return implicitWidth; })
        parent.Layout.maximumWidth = Qt.binding(function() { return bubbleMaxWidth + Kirigami.Units.largeSpacing * 2; })
    }
    Rectangle {
        id: replyLeftBorder
        width: Kirigami.Units.smallSpacing
        height: parent.height
        color: Kirigami.Theme.highlightColor
    }

    Kirigami.Avatar {
        id: avatatReply
        anchors.left: replyLeftBorder.right
        anchors.leftMargin: Kirigami.Units.smallSpacing
        width: Kirigami.Units.gridUnit
        height: Kirigami.Units.gridUnit
        sourceSize.width: width
        sourceSize.height: height
        Layout.alignment: Qt.AlignTop
        visible: Config.showAvatarInTimeline
        source: reply.author.avatarMediaId ? ("image://mxc/" + reply.author.avatarMediaId) : ""
        name: reply.author.name || ""
        color: reply.author.color
    }

    QQC2.Label {
        id: replyName
        anchors {
            left: avatatReply.right
            leftMargin: Kirigami.Units.smallSpacing
            right: parent.right
            rightMargin: Kirigami.Units.smallSpacing
        }
        text: reply.author.displayName
        color: reply.author.color
        elide: Text.ElideRight
    }

    TextDelegate {
        id: replyText
        anchors {
            left: avatatReply.right
            top: replyName.bottom
            leftMargin: Kirigami.Units.smallSpacing
            topMargin: Kirigami.Units.smallSpacing
            right: parent.right
            rightMargin: Kirigami.Units.smallSpacing
        }
        textMessage: reply.display
        textFormat: Text.RichText
        wrapMode: Text.WordWrap
    }
}
