// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: BSD-2-Clause

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtCore as Core

import org.kde.kirigami as Kirigami
import org.kde.kquickimageeditor as KQuickImageEditor

Kirigami.Page {
    id: rootEditorView

    property bool resizing: false
    required property string imagePath

    signal newPathChanged(string newPath)

    title: i18n("Edit")
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    function crop() {
        const ratioX = editImage.paintedWidth / editImage.nativeWidth;
        const ratioY = editImage.paintedHeight / editImage.nativeHeight;
        rootEditorView.resizing = false;
        imageDoc.crop(selectionTool.selectionX / ratioX, selectionTool.selectionY / ratioY, selectionTool.selectionWidth / ratioX, selectionTool.selectionHeight / ratioY);
    }

    actions: [
        Kirigami.Action {
            id: undoAction
            text: i18nc("@action:button Undo modification", "Undo")
            icon.name: "edit-undo"
            onTriggered: imageDoc.undo()
            visible: imageDoc.edited
        },
        Kirigami.Action {
            id: okAction
            text: i18nc("@action:button Accept image modification", "Accept")
            icon.name: "dialog-ok"
            onTriggered: {
                let newPath = Core.StandardPaths.writableLocation(Core.StandardPaths.CacheLocation) + "/" + (new Date()).getTime() + "." + imagePath.split('.').pop();
                if (imageDoc.saveAs(newPath)) {
                    newPathChanged(newPath);
                } else {
                    msg.type = Kirigami.MessageType.Error;
                    msg.text = i18n("Unable to save file. Check if you have the correct permission to edit the cache directory.");
                    msg.visible = true;
                }
            }
        }
    ]

    KQuickImageEditor.ImageItem {
        id: editImage
        // Assigning this to the contentItem and setting the padding causes weird positioning issues
        anchors.fill: parent
        anchors.margins: Kirigami.Units.gridUnit
        fillMode: KQuickImageEditor.ImageItem.PreserveAspectFit
        image: imageDoc.image

        Shortcut {
            sequence: StandardKey.Undo
            onActivated: undoAction.trigger()
        }

        Shortcut {
            sequences: [StandardKey.Save, "Enter"]
            onActivated: saveAction.trigger()
        }

        Shortcut {
            sequence: StandardKey.SaveAs
            onActivated: saveAsAction.trigger()
        }

        KQuickImageEditor.ImageDocument {
            id: imageDoc
            path: rootEditorView.imagePath
        }

        KQuickImageEditor.SelectionTool {
            id: selectionTool
            visible: rootEditorView.resizing
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
                    rootEditorView.crop();
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
                    icon.name: rootEditorView.resizing ? "dialog-cancel" : "transform-crop"
                    text: rootEditorView.resizing ? i18n("Cancel") : i18nc("@action:button Crop an image", "Crop")
                    onTriggered: {
                        resizeRectangle.width = editImage.paintedWidth;
                        resizeRectangle.height = editImage.paintedHeight;
                        resizeRectangle.x = editImage.horizontalPadding;
                        resizeRectangle.y = editImage.verticalPadding;
                        resizeRectangle.insideX = 100;
                        resizeRectangle.insideY = 100;
                        resizeRectangle.insideWidth = 100;
                        resizeRectangle.insideHeight = 100;
                        rootEditorView.resizing = !rootEditorView.resizing;
                    }
                },
                Kirigami.Action {
                    icon.name: "dialog-ok"
                    visible: rootEditorView.resizing
                    text: i18nc("@action:button Crop an image", "Crop")
                    onTriggered: rootEditorView.crop()
                },
                Kirigami.Action {
                    icon.name: "object-rotate-left"
                    text: i18nc("@action:button Rotate an image to the left", "Rotate left")
                    onTriggered: imageDoc.rotate(-90)
                    visible: !rootEditorView.resizing
                },
                Kirigami.Action {
                    icon.name: "object-rotate-right"
                    text: i18nc("@action:button Rotate an image to the right", "Rotate right")
                    onTriggered: imageDoc.rotate(90)
                    visible: !rootEditorView.resizing
                },
                Kirigami.Action {
                    icon.name: "object-flip-vertical"
                    text: i18nc("@action:button Mirror an image vertically", "Flip")
                    onTriggered: imageDoc.mirror(false, true)
                    visible: !rootEditorView.resizing
                },
                Kirigami.Action {
                    icon.name: "object-flip-horizontal"
                    text: i18nc("@action:button Mirror an image horizontally", "Mirror")
                    onTriggered: imageDoc.mirror(true, false)
                    visible: !rootEditorView.resizing
                }
            ]
        }
    }

    footer: Kirigami.InlineMessage {
        id: msg
        type: Kirigami.MessageType.Error
        showCloseButton: true
        visible: false
        position: Kirigami.InlineMessage.Position.Header
    }
}
