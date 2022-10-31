// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

ColumnLayout {
    id: _picker

    property string emojiCategory: "history"
    property var textArea
    readonly property var emojiModel: EmojiModel

    signal chosen(string emoji)

    spacing: 0

    QQC2.ScrollView {
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 2 + QQC2.ScrollBar.horizontal.height + 2 // for the focus line
        QQC2.ScrollBar.horizontal.height: QQC2.ScrollBar.horizontal.visible ? QQC2.ScrollBar.horizontal.implicitHeight : 0

        ListView {
            clip: true
            orientation: ListView.Horizontal

            model: ListModel {
                ListElement { label: "custom"; category: "custom" }
                ListElement { label: "⌛️"; category: "history" }
                ListElement { label: "😏"; category: "people" }
                ListElement { label: "🌲"; category: "nature" }
                ListElement { label: "🍛"; category: "food"}
                ListElement { label: "🚁"; category: "activity" }
                ListElement { label: "🚅"; category: "travel" }
                ListElement { label: "💡"; category: "objects" }
                ListElement { label: "🔣"; category: "symbols" }
                ListElement { label: "🏁"; category: "flags" }
            }

            delegate: QQC2.ItemDelegate {
                id: del

                required property string label
                required property string category

                width: contentItem.Layout.preferredWidth
                height: Kirigami.Units.gridUnit * 2

                contentItem: Kirigami.Heading {
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    level: del.label === "custom" ? 4 : 1

                    Layout.preferredWidth: del.label === "custom" ? implicitWidth + Kirigami.Units.largeSpacing : Kirigami.Units.gridUnit * 2

                    font.family: del.label === "custom" ? Kirigami.Theme.defaultFont.family : 'emoji'
                    text: del.label === "custom" ? i18n("Custom") : del.label
                }

                Rectangle {
                    anchors.bottom: parent.bottom

                    width: parent.width
                    height: 2

                    visible: emojiCategory === category

                    color: Kirigami.Theme.focusColor
                }

                onClicked: emojiCategory = category
            }
        }
    }

    Kirigami.Separator {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
    }

   QQC2.ScrollView {
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 8
        Layout.fillHeight: true

        GridView {
            cellWidth: Kirigami.Units.gridUnit * 2
            cellHeight: Kirigami.Units.gridUnit * 2

            clip: true

            model: {
                switch (emojiCategory) {
                case "custom":
                    return CustomEmojiModel
                case "history":
                    return emojiModel.history
                case "people":
                    return emojiModel.people
                case "nature":
                    return emojiModel.nature
                case "food":
                    return emojiModel.food
                case "activity":
                    return emojiModel.activity
                case "travel":
                    return emojiModel.travel
                case "objects":
                    return emojiModel.objects
                case "symbols":
                    return emojiModel.symbols
                case "flags":
                    return emojiModel.flags
                }
                return null
            }

            delegate: QQC2.ItemDelegate {
                width: Kirigami.Units.gridUnit * 2
                height: Kirigami.Units.gridUnit * 2

                contentItem: Kirigami.Heading {
                    level: 1
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.family: 'emoji'
                    text: modelData.isCustom ? "" : modelData.unicode
                }

                Image {
                    visible: modelData.isCustom
                    source: visible ? modelData.unicode : ""
                    anchors.fill: parent
                    anchors.margins: 2

                    sourceSize.width: width
                    sourceSize.height: height

                    Rectangle {
                        anchors.fill: parent
                        visible: parent.status === Image.Loading
                        radius: height/2
                        gradient: ShimmerGradient { }
                    }
                }

                onClicked: {
                    if (modelData.isCustom) {
                        chosen(modelData.shortname)
                    } else {
                        chosen(modelData.unicode)
                    }
                    emojiModel.emojiUsed(modelData)
                }
            }
        }
    }
}
