// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCard {
    id: root

    enum EmoticonType {
        Emojis,
        Stickers
    }

    property var emoticonType
    required property NeoChatConnection connection

    Flow {
        id: stickerFlow
        Layout.fillWidth: true
        Repeater {
            model: EmoticonFilterModel {
                id: emoticonFilterModel
                sourceModel: AccountEmoticonModel {
                    id: stickerModel
                    connection: root.connection
                }
                showStickers: root.emoticonType === EmoticonFormCard.Stickers
                showEmojis: root.emoticonType === EmoticonFormCard.Emojis
            }

            delegate: FormCard.AbstractFormDelegate {
                id: stickerDelegate

                width: stickerFlow.width / 4
                height: width

                onClicked: pageStack.pushDialogLayer(emoticonEditorPage, {
                    description: model.body ?? "",
                    index: model.index,
                    url: model.url,
                    shortcode: model.shortcode,
                    model: stickerModel,
                    proxyModel: emoticonFilterModel,
                    emoticonType: root.emoticonType
                }, {
                    title: root.emoticonType === EmoticonFormCard.Emojis ? i18nc("@title", "Edit Emoji") : i18nc("@title", "Edit Sticker")
                });

                contentItem: ColumnLayout {
                    Image {
                        source: model.url
                        Layout.fillWidth: true
                        sourceSize.height: parent.width * 0.8
                        fillMode: Image.PreserveAspectFit
                        autoTransform: true
                        Kirigami.Icon {
                            source: root.emoticonType === EmoticonFormCard.Emojis ? "preferences-desktop-emoticons" : "stickers"
                            anchors.fill: parent
                            visible: parent.status !== Image.Ready
                        }
                    }
                    QQC2.Label {
                        id: descriptionLabel
                        text: model.body ?? i18nc("As in 'This sticker/emoji has no description'", "No Description")
                        horizontalAlignment: Qt.AlignHCenter
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                }
                QQC2.Button {
                    icon.name: "edit-delete"
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: Kirigami.Units.smallSpacing
                    z: 2
                    onClicked: stickerModel.deleteEmoticon(emoticonFilterModel.mapToSource(emoticonFilterModel.index(model.index, 0)).row)
                }
            }
        }
        FormCard.AbstractFormDelegate {
            width: stickerFlow.width / 4
            height: width

            onClicked: pageStack.pushDialogLayer(emoticonEditorPage, {
                description: "",
                index: -1,
                url: "",
                shortcode: "",
                model: stickerModel,
                proxyModel: emoticonFilterModel,
                newEmoticon: true,
                emoticonType: root.emoticonType
            }, {
                title: root.emoticonType === EmoticonFormCard.Emojis ? i18nc("@title", "Add Emoji") : i18nc("@title", "Add Sticker")
            });
            contentItem: ColumnLayout {
                spacing: 0
                Kirigami.Icon {
                    source: "list-add"
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: root.emoticonType === EmoticonFormCard.Emojis ? i18n("Add Emoji") : i18n("Add Sticker")
                    horizontalAlignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                }
            }
        }
    }
}
