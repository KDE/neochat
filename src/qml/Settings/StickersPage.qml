// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18n("Stickers")
    leftPadding: 0
    rightPadding: 0

    ColumnLayout {
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("Stickers")
                }
                Flow {
                    id: stickerFlow
                    Layout.fillWidth: true
                    Repeater {
                        model: EmoticonFilterModel {
                            id: emoticonFilterModel
                            sourceModel: AccountEmoticonModel {
                                id: stickerModel
                                connection: Controller.activeConnection
                            }
                            showStickers: true
                            showEmojis: false
                        }

                        delegate: MobileForm.AbstractFormDelegate {
                            id: stickerDelegate

                            width: stickerFlow.width / 4
                            height: width

                            onClicked: pageSettingStack.pushDialogLayer(stickerEditorPage, {
                                description: model.body ?? "",
                                index: model.index,
                                url: model.url,
                                shortcode: model.shortcode,
                                model: stickerModel,
                                proxyModel: emoticonFilterModel
                            }, {
                                title: i18nc("@title", "Edit Sticker")
                            });

                            contentItem: ColumnLayout {
                                Image {
                                    source: model.url
                                    Layout.fillWidth: true
                                    sourceSize.height: parent.width * 0.8
                                    fillMode: Image.PreserveAspectFit
                                    autoTransform: true
                                    Kirigami.Icon {
                                        source: "stickers"
                                        anchors.fill: parent
                                        visible: parent.status !== Image.Ready
                                    }
                                }
                                QQC2.Label {
                                    id: descriptionLabel
                                    text: model.body ?? i18nc("As in 'This sticker has no description'", "No Description")
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
                    MobileForm.AbstractFormDelegate {
                        width: stickerFlow.width / 4
                        height: width

                        onClicked: pageSettingStack.pushDialogLayer(stickerEditorPage, {
                            description: "",
                            index: -1,
                            url: "",
                            shortcode: "",
                            model: stickerModel,
                            proxyModel: emoticonFilterModel,
                            newSticker: true
                        }, {
                            title: i18nc("@title", "Add Sticker")
                        });
                        contentItem: ColumnLayout {
                            spacing: 0
                            Kirigami.Icon {
                                source: "list-add"
                                Layout.fillWidth: true
                            }
                            QQC2.Label {
                                text: i18n("Add Sticker")
                                horizontalAlignment: Qt.AlignHCenter
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: stickerEditorPage
        StickerEditorPage {}
    }
}
