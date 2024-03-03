// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.kirigamiaddons.components as Components

import org.kde.neochat

Kirigami.ScrollablePage {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title", "Notifications")

    ListView {
        id: listView

        anchors.fill: parent
        verticalLayoutDirection: ListView.BottomToTop
        model: NotificationsModel {
            id: notificationsModel
            connection: root.connection
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: listView.count === 0
            text: notificationsModel.loading ? i18n("Loading…") : i18n("No Notifications")
        }

        footer: Kirigami.PlaceholderMessage {
            width: parent.width
            text: i18n("Loading…")
            visible: notificationsModel.nextToken.length > 0 && listView.count > 0
        }

        delegate: QQC2.ItemDelegate {
            width: parent?.width ?? 0

            onClicked: RoomManager.resolveResource(model.uri)
            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing

                Components.Avatar {
                    source: model.authorAvatar
                    name: model.authorName
                    implicitHeight: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
                    implicitWidth: implicitHeight

                    Layout.fillHeight: true
                    Layout.preferredWidth: height
                }

                ColumnLayout {
                    spacing: 0

                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter

                    QQC2.Label {
                        id: label

                        text: model.roomDisplayName
                        elide: Text.ElideRight
                        font.weight: Font.Normal
                        textFormat: Text.PlainText

                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                    }

                    QQC2.Label {
                        id: subtitle

                        text: model.text
                        elide: Text.ElideRight
                        font: Kirigami.Theme.smallFont
                        opacity: root.hasNotifications ? 0.9 : 0.7

                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    }
                }
            }
        }
    }
}
