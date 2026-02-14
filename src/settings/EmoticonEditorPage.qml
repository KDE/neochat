// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtCore as Core
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
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

    title: emoticonType === EmoticonFormCard.Stickers ? (newEmoticon ? i18nc("@title:window", "Add Sticker") : i18nc("@title:window", "Edit Sticker")) : (newEmoticon ? i18nc("@title:window", "Add Emoji") : i18nc("@title:window", "Edit Emoji"))

    FormCard.FormHeader {
        title: root.emoticonType === EmoticonFormCard.Stickers ? i18nc("@info:group", "Sticker") : i18nc("@info:group", "Emoji")
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
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 4
                    Layout.minimumHeight: Kirigami.Units.gridUnit * 4
                    source: root.url
                    sourceSize.width: Kirigami.Units.gridUnit * 4
                    sourceSize.height: Kirigami.Units.gridUnit * 4

                    Kirigami.Icon {
                        source: root.emoticonType === EmoticonFormCard.Stickers ? "stickers" : "preferences-desktop-emoticons"
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
                        property var fileDialog: null
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            if (fileDialog != null) {
                                return;
                            }
                            fileDialog = root.openFileDialog.createObject(QQC2.Overlay.overlay);
                            fileDialog.chosen.connect(function (receivedSource) {
                                mouseArea.fileDialog = null;
                                if (!receivedSource) {
                                    return;
                                }
                                parent.source = receivedSource;
                            });
                            fileDialog.onRejected.connect(function () {
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
            label: i18n("Shortcode:")
            text: root.shortcode
            onTextChanged: root.shortcode = text
        }
        FormCard.FormTextFieldDelegate {
            label: i18n("Description:")
            text: root.description
            onTextChanged: root.description = text
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            id: save
            text: i18n("Save")
            icon.name: "document-save"
            enabled: !root.newEmoticon || (image.source.toString().length > 0 && root.shortcode && root.description)
            onClicked: {
                if (root.newEmoticon) {
                    root.model.addEmoticon(image.source, root.shortcode, root.description, root.emoticonType === EmoticonFormCard.Stickers ? "sticker" : "emoticon");
                } else {
                    if (root.description !== root.description) {
                        root.model.setEmoticonBody(root.proxyModel.mapToSource(root.proxyModel.index(root.model.index, 0)).row, root.description);
                    }
                    if (root.shortcode !== root.shortcode) {
                        root.model.setEmoticonShortcode(root.proxyModel.mapToSource(root.proxyModel.index(root.model.index, 0)).row, root.shortcode);
                    }
                    if (image.source + "" !== root.url) {
                        root.model.setEmoticonImage(root.proxyModel.mapToSource(root.proxyModel.index(root.model.index, 0)).row, image.source);
                    }
                }
                root.closeDialog();
            }
        }
    }
    property Component openFileDialog: Component {
        OpenFileDialog {
            currentFolder: Core.StandardPaths.standardLocations(Core.StandardPaths.PicturesLocation)[0]
            parentWindow: root.Window.window
        }
    }
}
