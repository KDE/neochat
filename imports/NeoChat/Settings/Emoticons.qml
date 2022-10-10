// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import Qt.labs.platform 1.1

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Settings 1.0

import NeoChat.Component 1.0 as Components
import NeoChat.Dialog 1.0

Kirigami.ScrollablePage {
    title: i18nc("@title:window", "Custom Emojis")

    ListView {
        anchors.fill: parent

        model: CustomEmojiModel

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            text: i18n("No custom inline stickers found")
            visible: parent.model.count === 0
        }

        delegate: Kirigami.BasicListItem {
            id: del

            required property string name
            required property url imageURL

            text: name
            reserveSpaceForSubtitle: true

            leading: Image {
                width: height
                sourceSize.width: width
                sourceSize.height: height
                source: imageURL

                Rectangle {
                    anchors.fill: parent
                    visible: parent.status === Image.Loading
                    radius: height/2
                    gradient: Components.ShimmerGradient { }
                }
            }

            trailing: QQC2.ToolButton {
                width: height
                icon.name: "delete"
                onClicked: emojiModel.removeEmoji(del.name)
            }
        }
    }

    footer: QQC2.ToolBar {
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.ActionToolBar {
            id: emojiCreator
            alignment: Qt.AlignRight
            rightPadding: Kirigami.Units.smallSpacing
            width: parent.width
            flat: false
            property string name
            actions: [
                Kirigami.Action {
                    displayComponent: QQC2.TextField {
                        id: emojiField
                        placeholderText: i18n("new_emoji_name_here")

                        validator: RegularExpressionValidator {
                            regularExpression: /[a-zA-Z_0-9]*/
                        }
                        onTextChanged: emojiCreator.name = text
                    }
                },
                Kirigami.Action {
                    text: i18n("Add Emoji...")

                    enabled: emojiCreator.name.length > 0
                    property var fileDialog: null
                    icon.name: 'list-add'

                    onTriggered: {
                        if (this.fileDialog !== null) {
                            return;
                        }

                        this.fileDialog = openFileDialog.createObject(QQC2.Overlay.overlay)

                        this.fileDialog.chosen.connect((url) => {
                            CustomEmojiModel.addEmoji(emojiCreator.name, url)
                            this.fileDialog = null
                        })
                        this.fileDialog.onRejected.connect(() => {
                            this.fileDialog = null
                        })
                        this.fileDialog.open()
                    }
                }
            ]
        }
    }

    Component {
        id: openFileDialog

        OpenFileDialog {
            folder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        }
    }
}
