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
                anchors.bottom: parent.bottom
                anchors.right: parent.right

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
                level: 1
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

    QQC2.ScrollView {
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.rightMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.maximumHeight: Math.min(topicText.contentHeight, Kirigami.Units.gridUnit * 15)

        // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        QQC2.TextArea {
            id: topicText
            padding: 0
            text: room && room.topic ? room.topic.replace(replaceLinks, "<a href=\"$1\">$1</a>") : i18n("No Topic")
            readonly property var replaceLinks: /(http[s]?:\/\/[^ \r\n]*)/g
            textFormat: TextEdit.MarkdownText
            wrapMode: Text.Wrap
            selectByMouse: true
            color: Kirigami.Theme.textColor
            selectedTextColor: Kirigami.Theme.highlightedTextColor
            selectionColor: Kirigami.Theme.highlightColor
            onLinkActivated: UrlHelper.openUrl(link)
            readOnly: true
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
            }
            background: Item {}
            Component.onCompleted: EmojiFixer.addTextDocument(topicText.textDocument)
        }
    }

}
