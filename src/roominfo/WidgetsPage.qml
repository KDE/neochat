// SPDX-FileCopyrightText: 2025 Arno Rehn <arno@arnorehn.de>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat.libneochat

Kirigami.ScrollablePage {
    id: root

    required property NeoChatRoom room

    title: i18nc("@title", "Extensions")

    ListView {
        id: extView
        visible: extView.count !== 0

        currentIndex: -1

        model: WidgetModel {
            room: root.room
        }

        delegate: Delegates.RoundedItemDelegate {
            id: del

            required text
            required property url url
            required property string type

            // Can we actually use the jitsi logo without being infringing any
            // trademarks?
            icon.name: type === "jitsi" ? "meeting-attending"
                        : type === "m.etherpad" ? "document-share"
                        : ""

            contentItem: Delegates.SubtitleContentItem {
                iconItem.visible: true
                itemDelegate: del
                subtitle: del.url
                labelItem.textFormat: Text.PlainText
            }

            onClicked: Qt.openUrlExternally(url)
        }
    }

    Kirigami.PlaceholderMessage {
        text: i18nc("@info:placeholder", "There are no extensions in this room.")
        visible: extView.count === 0
        anchors.centerIn: parent
    }
}
