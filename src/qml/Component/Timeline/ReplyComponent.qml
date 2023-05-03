// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Item {
    id: replyComponent

    signal replyClicked()

    property var name
    property alias avatar: replyAvatar.source
    property var color
    property var mediaInfo

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
            Layout.maximumHeight: loader.item && (reply.type == MessageEventModel.Image || reply.type == MessageEventModel.Sticker) ? loader.item.height : -1
            Layout.columnSpan: 2

            sourceComponent: {
                switch (reply.type) {
                    case MessageEventModel.Image:
                    case MessageEventModel.Sticker:
                        return imageComponent;
                    case MessageEventModel.Message:
                    case MessageEventModel.Notice:
                        return textComponent;
                    case MessageEventModel.File:
                    case MessageEventModel.Video:
                    case MessageEventModel.Audio:
                        return mimeComponent;
                    case MessageEventModel.Encrypted:
                        return encryptedComponent;
                    default:
                        return textComponent;
                }
            }
        }
        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }
        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: replyComponent.replyClicked()
        }
    }

    Component {
        id: textComponent
        RichLabel {
            textMessage: reply.display
            textFormat: Text.RichText

            HoverHandler {
                enabled: !hoveredLink
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                enabled: !hoveredLink
                acceptedButtons: Qt.LeftButton
                onTapped: replyComponent.replyClicked()
            }
        }
    }
    Component {
        id: imageComponent
        Image {
            id: image

            property var imageWidth: {
                if (replyComponent.mediaInfo.width > 0) {
                    return replyComponent.mediaInfo.width;
                } else {
                    return sourceSize.width;
                }
            }
            property var imageHeight: {
                if (replyComponent.mediaInfo.height > 0) {
                    return replyComponent.mediaInfo.height;
                } else {
                    return sourceSize.height;
                }
            }

            readonly property var aspectRatio: imageWidth / imageHeight

            height: width / aspectRatio
            fillMode: Image.PreserveAspectFit
            source: mediaInfo.source
        }
    }
    Component {
        id: mimeComponent
        MimeComponent {
            mimeIconSource: replyComponent.mediaInfo.mimeIcon
            label: reply.display
            subLabel: reply.type === MessageEventModel.File ? Controller.formatByteSize(replyComponent.mediaInfo.size) : Controller.formatDuration(replyComponent.mediaInfo.duration)
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
