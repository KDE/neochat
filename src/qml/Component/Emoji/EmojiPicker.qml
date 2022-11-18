// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

ColumnLayout {
    id: _picker

    property var emojiCategory: EmojiModel.History
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

            model: EmojiModel.categories
            delegate: QQC2.ItemDelegate {
                id: del

                width: contentItem.Layout.preferredWidth
                height: Kirigami.Units.gridUnit * 2

                contentItem: Kirigami.Heading {
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    level: modelData.category === EmojiModel.Custom ? 4 : 1

                    Layout.preferredWidth: modelData.category === EmojiModel.Custom ? implicitWidth + Kirigami.Units.largeSpacing : Kirigami.Units.gridUnit * 2

                    font.family: modelData.category === EmojiModel.Custom ? Kirigami.Theme.defaultFont.family : 'emoji'
                    text: modelData.category === EmojiModel.Custom ? i18n("Custom") : modelData.emoji
                }

                Rectangle {
                    anchors.bottom: parent.bottom

                    width: parent.width
                    height: 2

                    visible: _picker.emojiCategory === modelData.category

                    color: Kirigami.Theme.focusColor
                }

                onClicked: _picker.emojiCategory = modelData.category
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

            model: _picker.emojiCategory === EmojiModel.Custom ? CustomEmojiModel : EmojiModel.emojis(_picker.emojiCategory)

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
                        chosen(modelData.shortName)
                    } else {
                        chosen(modelData.unicode)
                    }
                    emojiModel.emojiUsed(modelData)
                }
            }
        }
    }
}
