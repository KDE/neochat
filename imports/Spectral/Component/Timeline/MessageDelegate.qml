/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-or-later
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12

import org.kde.kirigami 2.13 as Kirigami

import Spectral 0.1
import Spectral.Setting 0.1
import Spectral.Component 2.0

RowLayout {
    default property alias innerObject : column.children

    readonly property bool sentByMe: author.isLocalUser
    readonly property bool darkBackground: !sentByMe
    readonly property bool replyVisible: reply ?? false
    readonly property bool failed: marks == EventStatus.SendingFailed
    readonly property color authorColor: eventType == "notice" ? MPalette.primary : author.color
    readonly property color replyAuthorColor: replyVisible ? reply.author.color : MPalette.accent

    property alias mouseArea: controlContainer.children

    signal saveFileAs()
    signal openExternally()

    id: root

    spacing: Kirigami.Units.largeSpacing
    Layout.leftMargin: Kirigami.Units.smallSpacing
    Layout.rightMargin: Kirigami.Units.smallSpacing

    Kirigami.Avatar {
        Layout.minimumWidth: Kirigami.Units.iconSizes.medium
        Layout.minimumHeight: Kirigami.Units.iconSizes.medium
        Layout.maximumWidth: Kirigami.Units.iconSizes.medium
        Layout.maximumHeight: Kirigami.Units.iconSizes.medium

        Layout.alignment: Qt.AlignTop

        visible: showAuthor
        name: author.displayName
        source: author.avatarMediaId ? "image://mxc/" + author.avatarMediaId : ""
        color: author.color
    }

    Item {
        Layout.minimumWidth: Kirigami.Units.iconSizes.medium
        Layout.preferredHeight: 1

        visible: !showAuthor
    }


    QQC2.Control {
        id: controlContainer
        Layout.fillWidth: true
        contentItem: ColumnLayout {
            id: column
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                Layout.fillWidth: true

                visible: showAuthor

                text: author.displayName
                font.bold: true
                color: author.color
                wrapMode: Text.Wrap
            }

            RowLayout {
                Layout.fillWidth: true

                visible: replyVisible

                Rectangle {
                    Layout.preferredWidth: Kirigami.Units.smallSpacing
                    Layout.fillHeight: true

                    color: Kirigami.Theme.highlightColor
                }

                Kirigami.Avatar {
                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                    Layout.alignment: Qt.AlignTop

                    source: replyVisible && reply.author.avatarMediaId ? "image://mxc/" + reply.author.avatarMediaId : ""
                    name: replyVisible ? reply.author.displayName : "H"
                    color: replyVisible ? reply.author.color : Kirigami.Theme.highlightColor
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    QQC2.Label {
                        Layout.fillWidth: true

                        text: replyVisible ? reply.author.displayName : ""
                        color: replyVisible ? reply.author.color: null
                        wrapMode: Text.Wrap
                    }

                    Text {
                        Layout.fillWidth: true

                        text: replyVisible ? reply.display : ""

                        color: Kirigami.Theme.textColor
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        textFormat: Text.RichText
                    }
                }
            }

            ReactionDelegate {
                Layout.fillWidth: true

                Layout.topMargin: 0
                Layout.bottomMargin: 8
                Layout.leftMargin: 16
                Layout.rightMargin: 16
            }
        }
    }
}
