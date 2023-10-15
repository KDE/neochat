// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.platform
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    required property string description
    required property string index
    required property string url
    required property string shortcode
    required property var model
    required property var proxyModel
    property bool newEmoticon: false
    required property var emoticonType

    title: emoticonType === EmoticonFormCard.Stickers ? (newEmoticon ? i18nc("@title", "Add Sticker") : i18nc("@title", "Edit Sticker"))
            : (newEmoticon ? i18nc("@title", "Add Emoji") : i18nc("@title", "Edit Emoji"))

    FormCard.FormHeader {
        title: emoticonType === EmoticonFormCard.Stickers ? i18n("Sticker") : i18n("Emoji")
    }
    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            background: Item {}
            contentItem: RowLayout {
                Item {
                    Layout.fillWidth: true
                }
                Image {
                    id: image
                    Layout.alignment: Qt.AlignRight
                    source: root.url
                    sourceSize.width: Kirigami.Units.gridUnit * 4
                    sourceSize.height: Kirigami.Units.gridUnit * 4
                    width: Kirigami.Units.gridUnit * 4
                    height: Kirigami.Units.gridUnit * 4

                    Kirigami.Icon {
                        source: emoticonType === EmoticonFormCard.Stickers ? "stickers" : "preferences-desktop-emoticons"
                        anchors.fill: parent
                        visible: parent.status !== Image.Ready
                    }

                    QQC2.Button {
                        icon.name: "edit-entry"
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        onClicked: mouseArea.clicked()
                        text: image.source != "" ? i18n("Change Image") : i18n("Set Image")
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        display: QQC2.Button.IconOnly
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        property var fileDialog: null;
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            if (fileDialog != null) {
                                return;
                            }

                            fileDialog = openFileDialog.createObject(QQC2.ApplicationWindow.Overlay)
                            fileDialog.chosen.connect(function(receivedSource) {
                                mouseArea.fileDialog = null;
                                if (!receivedSource) {
                                    return;
                                }
                                parent.source = receivedSource;
                            });
                            fileDialog.onRejected.connect(function() {
                                mouseArea.fileDialog = null;
                            });
                            fileDialog.open();
                        }
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
            }
        }
        FormCard.FormTextFieldDelegate {
            id: shortcode
            label: i18n("Shortcode:")
            text: root.shortcode
        }
        FormCard.FormTextFieldDelegate {
            id: description
            label: i18n("Description:")
            text: root.description
        }
        FormCard.FormButtonDelegate {
            id: save
            text: i18n("Save")
            icon.name: "document-save"
            enabled: !root.newEmoticon || (image.source.toString().length > 0 && shortcode.text && description.text)
            onClicked: {
                if (root.newEmoticon) {
                    model.addEmoticon(image.source, shortcode.text, description.text, emoticonType === EmoticonFormCard.Stickers ? "sticker" : "emoticon")
                } else {
                    if (description.text !== root.description) {
                        root.model.setEmoticonBody(proxyModel.mapToSource(proxyModel.index(model.index, 0)).row, description.text)
                    }
                    if (shortcode.text !== root.shortcode) {
                        root.model.setEmoticonShortcode(proxyModel.mapToSource(proxyModel.index(model.index, 0)).row, shortcode.text)
                    }
                    if (image.source + "" !== root.url) {
                        root.model.setEmoticonImage(proxyModel.mapToSource(proxyModel.index(model.index, 0)).row, image.source)
                    }
                }
                root.closeDialog()
            }
        }
    }
    property Component openFileDialog: Component {
        id: openFileDialog

        OpenFileDialog {
            currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
            parentWindow: root.Window.window
        }
    }
}
