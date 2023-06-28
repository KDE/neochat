// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0

ColumnLayout {
    id: root

    Layout.fillWidth: true

    RowLayout {
        Layout.fillWidth: true
        Layout.leftMargin: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Avatar {
            Layout.preferredWidth: Kirigami.Units.gridUnit * 3.5
            Layout.preferredHeight: Kirigami.Units.gridUnit * 3.5

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
                type: Kirigami.Heading.Type.Primary
                wrapMode: QQC2.Label.Wrap
                text: room ? room.displayName : i18n("No name")
                textFormat: Text.PlainText
            }

            Kirigami.SelectableLabel {
                Layout.fillWidth: true
                textFormat: TextEdit.PlainText
                text: room && room.canonicalAlias ? room.canonicalAlias : i18n("No Canonical Alias")
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
        onLinkActivated: UrlHelper.openUrl(link)
    }
}
