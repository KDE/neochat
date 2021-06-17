// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Settings 1.0

import NeoChat.Component 1.0 as Components
import NeoChat.Dialog 1.0

Kirigami.Page {

    leftPadding: pageSettingStack.wideMode ? Kirigami.Units.gridUnit : 0
    topPadding: pageSettingStack.wideMode ? Kirigami.Units.gridUnit : 0
    bottomPadding: pageSettingStack.wideMode ? Kirigami.Units.gridUnit : 0
    rightPadding: pageSettingStack.wideMode ? Kirigami.Units.gridUnit : 0

    ColumnLayout {
        id: column
        anchors.fill: parent

        Connections {
            target: pageSettingStack
            onWideModeChanged: scroll.background.visible = pageSettingStack.wideMode
        }

        QQC2.ScrollView {
            id: scroll
            Component.onCompleted: background.visible = pageSettingStack.wideMode
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                clip: true
                model: CustomEmojiModel {
                    id: emojiModel

                    connection: Controller.activeConnection
                }

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
        }

        Loader {
            active: pageSettingStack.wideMode
            sourceComponent: addEmojiComponent
            Layout.fillWidth: true
        }
    }

    footer: QQC2.ToolBar {
        id: toolbar
        width: parent.width
        visible: !pageSettingStack.wideMode
        height: visible ? implicitHeight : 0
        contentItem: Loader {
            active: !pageSettingStack.wideMode
            sourceComponent: addEmojiComponent
        }
    }

    Component {
        id: openFileDialog

        OpenFileDialog {
            folder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        }
    }

    property Component addEmojiComponent: RowLayout {
        Item {
            Layout.fillWidth: Qt.application.layoutDirection == Qt.LeftToRight
        }

        QQC2.TextField {
            id: emojiField
            placeholderText: i18n("new_emoji_name_here")

            validator: RegularExpressionValidator {
                regularExpression: /[a-zA-Z_0-9]*/
            }
        }

        QQC2.Button {
            text: i18n("Add Emoji...")

            enabled: emojiField.text != ""
            property var fileDialog: null

            onClicked: {
                if (this.fileDialog != null) {
                    return;
                }

                this.fileDialog = openFileDialog.createObject(QQC2.Overlay.overlay)

                this.fileDialog.chosen.connect((url) => {
                    emojiModel.addEmoji(emojiField.text, url)
                    this.fileDialog = null
                })
                this.fileDialog.onRejected.connect(() => {
                    rej()
                    this.fileDialog = null
                })
                this.fileDialog.open()
            }
        }

        Item {
            Layout.fillWidth: Qt.application.layoutDirection == Qt.RightToLeft
        }
    }
}
