// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents

import org.kde.neochat 1.0

ColumnLayout {
    id: root

    Layout.fillWidth: true

    RowLayout {
        Layout.fillWidth: true
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.bottomMargin: Kirigami.Units.largeSpacing

        spacing: Kirigami.Units.largeSpacing

        KirigamiComponents.Avatar {
            Layout.preferredWidth: Kirigami.Units.iconSizes.large
            Layout.preferredHeight: Kirigami.Units.iconSizes.large

            name: room ? room.displayName : ""
            source: room && room.avatarMediaId ? ("image://mxc/" +  room.avatarMediaId) : ""

            Rectangle {
                visible: room.usesEncryption
                color: Kirigami.Theme.backgroundColor

                width: Kirigami.Units.gridUnit
                height: Kirigami.Units.gridUnit

                anchors {
                    bottom: parent.bottom
                    right: parent.right
                }

                radius: Math.round(width / 2)

                Kirigami.Icon {
                    source: "channel-secure-symbolic"
                    anchors.fill: parent
                }
            }
        }


        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 0

            Kirigami.Heading {
                Layout.fillWidth: true
                text: room ? room.displayName : i18n("No name")
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
            }

            Kirigami.SelectableLabel {
                Layout.fillWidth: true
                font: Kirigami.Theme.smallFont
                textFormat: TextEdit.PlainText
                visible: room && room.canonicalAlias
                text: room && room.canonicalAlias ? room.canonicalAlias : ""
            }
        }
    }

    Kirigami.SelectableLabel {
        Layout.fillWidth: true
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.rightMargin: Kirigami.Units.largeSpacing

        text: room && room.topic ? room.topic.replace(replaceLinks, "<a href=\"$1\">$1</a>") : i18n("No Topic")
        readonly property var replaceLinks: /(http[s]?:\/\/[^ \r\n]*)/g
        textFormat: TextEdit.MarkdownText
        wrapMode: Text.Wrap
        onLinkActivated: link => UrlHelper.openUrl(link)
    }
}
