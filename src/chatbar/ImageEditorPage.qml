// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: BSD-2-Clause

import QtQuick
import QtQuick.Controls as QQC2
import QtCore as Core
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kquickimageeditor as KQuickImageEditor

Kirigami.Page {
    id: root

    property bool resizing: false
    required property string imagePath

    signal newPathChanged(string newPath)

    title: i18nc("@window:title", "Edit Image")
    globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    function crop() {
        const ratioX = editImage.paintedWidth / editImage.nativeWidth;
        const ratioY = editImage.paintedHeight / editImage.nativeHeight;
        root.resizing = false;
        imageDoc.crop(selectionTool.selectionX / ratioX, selectionTool.selectionY / ratioY, selectionTool.selectionWidth / ratioX, selectionTool.selectionHeight / ratioY);
    }

    footer: QQC2.ToolBar {
        QQC2.DialogButtonBox {
            anchors.fill: parent

            QQC2.Button {
                id: saveButton

                text: i18nc("@action:button Accept image modification", "Save")
                icon.name: "document-save"

                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.ApplyRole

                onClicked: {
                    let newPath = Core.StandardPaths.writableLocation(Core.StandardPaths.CacheLocation) + "/" + (new Date()).getTime() + "." + root.imagePath.split('.').pop();
                    if (imageDoc.saveAs(newPath)) {
                        root.newPathChanged(newPath);
                    } else {
                        // TODO: is this a thing that will ever actually happen?
                        console.warn("Unable to save file. Check if you have the correct permission to edit the cache directory.");
                    }
                }
            }
            QQC2.Button {
                id: undoButton

                text: i18nc("@action:button Undo modification", "Undo")
                icon.name: "edit-undo"
                enabled: imageDoc.edited

                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.ResetRole

                onClicked: imageDoc.undo()
            }
        }
    }

    KQuickImageEditor.ImageItem {
        id: editImage
        // Assigning this to the contentItem and setting the padding causes weird positioning issues
        anchors.fill: parent
        anchors.margins: Kirigami.Units.gridUnit
        fillMode: KQuickImageEditor.ImageItem.PreserveAspectFit
        image: imageDoc.image

        Shortcut {
            sequence: StandardKey.Undo
            onActivated: undoButton.click()
        }

        Shortcut {
            sequences: [StandardKey.Save, "Enter"]
            onActivated: saveButton.trigger()
        }

        KQuickImageEditor.ImageDocument {
            id: imageDoc
            path: root.imagePath
        }

        KQuickImageEditor.SelectionTool {
            id: selectionTool
            visible: root.resizing
            width: editImage.paintedWidth
            height: editImage.paintedHeight
            x: editImage.horizontalPadding
            y: editImage.verticalPadding
            KQuickImageEditor.CropBackground {
                anchors.fill: parent
                z: -1
                insideX: selectionTool.selectionX
                insideY: selectionTool.selectionY
                insideWidth: selectionTool.selectionWidth
                insideHeight: selectionTool.selectionHeight
            }
            Connections {
                target: selectionTool.selectionArea
                function onDoubleClicked() {
                    root.crop();
                }
            }
        }

        onImageChanged: {
            selectionTool.selectionX = 0;
            selectionTool.selectionY = 0;
            selectionTool.selectionWidth = Qt.binding(() => selectionTool.width);
            selectionTool.selectionHeight = Qt.binding(() => selectionTool.height);
        }
    }

    header: QQC2.ToolBar {
        contentItem: Kirigami.ActionToolBar {
            id: actionToolBar
            display: QQC2.Button.TextBesideIcon
            actions: [
                Kirigami.Action {
                    icon.name: "dialog-ok"
                    visible: root.resizing
                    text: i18nc("@action:button Crop an image", "Crop")
                    onTriggered: root.crop()
                },
                Kirigami.Action {
                    icon.name: "object-rotate-left"
                    text: i18nc("@action:button Rotate an image to the left", "Rotate left")
                    onTriggered: imageDoc.rotate(-90)
                    visible: !root.resizing
                },
                Kirigami.Action {
                    icon.name: "object-rotate-right"
                    text: i18nc("@action:button Rotate an image to the right", "Rotate right")
                    onTriggered: imageDoc.rotate(90)
                    visible: !root.resizing
                },
                Kirigami.Action {
                    icon.name: "object-flip-vertical"
                    text: i18nc("@action:button Mirror an image vertically", "Flip")
                    onTriggered: imageDoc.mirror(false, true)
                    visible: !root.resizing
                },
                Kirigami.Action {
                    icon.name: "object-flip-horizontal"
                    text: i18nc("@action:button Mirror an image horizontally", "Mirror")
                    onTriggered: imageDoc.mirror(true, false)
                    visible: !root.resizing
                }
            ]
        }
    }
}
