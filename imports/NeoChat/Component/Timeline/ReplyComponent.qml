// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component.Timeline 1.0

Item {
    id: replyComponent

    signal replyClicked()

    property var name
    property alias avatar: replyAvatar.source
    property var color

    implicitWidth: mainLayout.implicitWidth
    implicitHeight: mainLayout.implicitHeight

    GridLayout {
        id: mainLayout

        anchors.fill: parent

        rows: 2
        columns: 3
        rowSpacing: Kirigami.Units.smallSpacing
        columnSpacing: Kirigami.Units.largeSpacing

        Rectangle {
            id: verticalBorder

            Layout.fillHeight: true
            Layout.rowSpan: 2

            implicitWidth: Kirigami.Units.smallSpacing
            color: replyComponent.color
        }
        Kirigami.Avatar {
            id: replyAvatar

            implicitWidth: Kirigami.Units.iconSizes.small
            implicitHeight: Kirigami.Units.iconSizes.small

            name: replyComponent.name || ""
            color: replyComponent.color
        }
        QQC2.Label {
            Layout.fillWidth: true

            color: replyComponent.color
            text: replyComponent.name
            elide: Text.ElideRight
        }
        Loader {
            id: loader

            Layout.fillWidth: true
            Layout.columnSpan: 2

            sourceComponent: {
                switch (reply.type) {
                    case "image":
                    case "sticker":
                        return imageComponent;
                    case "message":
                    case "notice":
                        return textComponent;
                    case "file":
                    case "video":
                    case "audio":
                        return mimeComponent;
                    case "encrypted":
                        return encryptedComponent;
                    default:
                        return textComponent;
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            replyComponent.replyClicked()
        }
    }

    Component {
        id: textComponent
        RichLabel {
            textMessage: reply.display
            textFormat: Text.RichText
        }
    }
    Component {
        id: imageComponent
        Image {
            id: image
            readonly property var content: reply.content
            readonly property bool isThumbnail: !(content.info.thumbnail_info == null || content.thumbnailMediaId == null)
            readonly property var info: content.info
            readonly property string mediaId: isThumbnail ? content.thumbnailMediaId : content.mediaId
            source: "image://mxc/" + mediaId
        }
    }
    Component {
        id: mimeComponent
        MimeComponent {
            mimeIconSource: reply.content.info.mimetype.replace("/", "-")
            label: reply.display
            subLabel: reply.type === "file" ? Controller.formatByteSize(reply.content.info ? reply.content.info.size : 0) : Controller.formatDuration(reply.content.info.duration)
        }
    }
    Component {
        id: encryptedComponent
        RichLabel {
            textMessage: i18n("This message is encrypted and the sender has not shared the key with this device.")
            textFormat: Text.RichText
        }
    }
}
