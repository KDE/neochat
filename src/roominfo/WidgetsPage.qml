// SPDX-FileCopyrightText: 2025 Arno Rehn <arno@arnorehn.de>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat.libneochat

Kirigami.ScrollablePage {
    id: root

    required property NeoChatRoom room

    title: i18nc("@title", "Extensions")

    ListView {
        id: extView

        currentIndex: -1

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            icon.name: "extension-symbolic"
            text: i18nc("@placeholder", "No extensions")
            visible: extView.count === 0
        }

        model: WidgetModel {
            id: widgetModel
            room: root.room
        }

        delegate: QQC2.ItemDelegate {
            id: del

            required text
            required property url url
            required property string type
            required property int index

            width: ListView.view.width

            // Can we actually use the jitsi logo without being infringing any
            // trademarks?
            icon.name: type === "jitsi" ? "meeting-attending"
                        : type === "m.etherpad" ? "document-share"
                        : ""
            icon.width: Kirigami.Units.iconSizes.smallMedium
            icon.height: Kirigami.Units.iconSizes.smallMedium

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Kirigami.IconTitleSubtitle {
                    Layout.fillWidth: true

                    icon: icon.fromControlsIcon(del.icon)
                    title: del.text
                    subtitle: del.url
                    selected: del.highlighted || del.down
                    font: del.font

                    // TODO: Remove when we can depend on Kirigami 6.31
                    Component.onCompleted: {
                        if (textFormat !== undefined) {
                            textFormat = Text.PlainText;
                        }
                    }
                }

                QQC2.ToolButton {
                    action: Kirigami.Action {
                        icon.name: "delete-symbolic"
                        tooltip: i18nc("@action:button", "Remove widget")
                        onTriggered: widgetModel.removeWidget(del.index)
                    }
                }
            }

            onClicked: Qt.openUrlExternally(url)
        }
    }
}
