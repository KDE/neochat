// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Page 1.0

Loader {
    id: attachmentPaneLoader

    readonly property var attachmentMimetype: FileType.mimeTypeForUrl(attachmentPaneLoader.attachmentPath)
    readonly property bool hasImage: attachmentMimetype.valid && FileType.supportedImageFormats.includes(attachmentMimetype.preferredSuffix)
    readonly property string attachmentPath: currentRoom.chatBoxAttachmentPath
    readonly property string baseFileName: attachmentPath.substring(attachmentPath.lastIndexOf('/') + 1, attachmentPath.length)

    active: visible
    sourceComponent: Component {
        Pane {
            id: attachmentPane
            Kirigami.Theme.colorSet: Kirigami.Theme.View

            contentItem: Item {
                property real spacing: attachmentPane.spacing > 0 ? attachmentPane.spacing : toolBar.spacing
                implicitWidth: Math.max(image.implicitWidth, imageBusyIndicator.implicitWidth, fileInfoLayout.implicitWidth, toolBar.implicitWidth)
                implicitHeight: Math.max(
                    (hasImage ? Math.max(image.preferredHeight, imageBusyIndicator.implicitHeight) + spacing : 0)
                        + fileInfoLayout.implicitHeight,
                    toolBar.implicitHeight
                )

                Image {
                    id: image
                    property real preferredHeight: Math.min(implicitHeight, Kirigami.Units.gridUnit * 8)
                    height: preferredHeight
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        bottom: fileInfoLayout.top
                        bottomMargin: parent.spacing
                    }
                    width: Math.min(implicitWidth, attachmentPane.availableWidth)
                    asynchronous: true
                    cache: false // Cache is not needed. Images will rarely be shown repeatedly.
                    smooth: height === preferredHeight && parent.height === parent.implicitHeight // Don't smooth until height animation stops
                    source: hasImage ? attachmentPaneLoader.attachmentPath : ""
                    visible: hasImage
                    fillMode: Image.PreserveAspectFit

                    onSourceChanged: {
                        // Reset source size height, which affect implicitHeight
                        sourceSize.height = -1
                    }

                    onSourceSizeChanged: {
                        if (implicitHeight > Kirigami.Units.gridUnit * 8) {
                            // This can save a lot of RAM when loading large images.
                            // It also improves visual quality for large images.
                            sourceSize.height = Kirigami.Units.gridUnit * 8
                        }
                    }

                    Behavior on height {
                        NumberAnimation {
                            property: "height"
                            duration: Kirigami.Units.shortDuration
                            easing.type: Easing.OutCubic
                        }
                    }
                }

                BusyIndicator {
                    id: imageBusyIndicator
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        top: parent.top
                        bottom: fileInfoLayout.top
                        bottomMargin: parent.spacing
                    }
                    visible: running
                    running: image.visible && image.progress < 1
                }

                RowLayout {
                    id: fileInfoLayout
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: undefined
                    anchors.bottom: parent.bottom
                    spacing: parent.spacing

                    Kirigami.Icon {
                        id: mimetypeIcon
                        implicitHeight: Kirigami.Units.fontMetrics.roundedIconSize(fileLabel.implicitHeight)
                        implicitWidth: implicitHeight
                        source: attachmentMimetype.iconName
                    }

                    Label {
                        id: fileLabel
                        text: baseFileName
                    }

                    states: State {
                        when: !hasImage
                        AnchorChanges {
                            target: fileInfoLayout
                            anchors.bottom: undefined
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                // Using a toolbar to get a button spacing consistent with what the QQC2 style normally has
                // Also has some accessibility info
                ToolBar {
                    id: toolBar
                    width: parent.width
                    anchors.top: parent.top

                    leftPadding: 0
                    rightPadding: 0
                    topPadding: 0
                    bottomPadding: 0

                    Kirigami.Theme.inherit: true
                    Kirigami.Theme.colorSet: Kirigami.Theme.View

                    contentItem: RowLayout {
                        spacing: parent.spacing
                        Label {
                            Layout.leftMargin: -attachmentPane.leftPadding
                            Layout.topMargin: -attachmentPane.topPadding
                            leftPadding: cancelAttachmentButton.leftPadding + 1 + attachmentPane.leftPadding
                            rightPadding: cancelAttachmentButton.rightPadding + 1
                            topPadding: cancelAttachmentButton.topPadding + attachmentPane.topPadding
                            bottomPadding: cancelAttachmentButton.bottomPadding
                            text: i18n("Attachment:")
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                            background: Kirigami.ShadowedRectangle {
                                property real cornerRadius: cancelAttachmentButton.background.hasOwnProperty("radius") ?
                                    Math.min(cancelAttachmentButton.background.radius, height/2) : 0
                                corners.bottomLeftRadius: toolBar.mirrored ? cornerRadius : 0
                                corners.bottomRightRadius: toolBar.mirrored ? 0 : cornerRadius
                                color: Kirigami.Theme.backgroundColor
                                opacity: 0.75
                            }
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        ToolButton {
                            id: editImageButton
                            visible: hasImage
                            icon.name: "document-edit"
                            text: i18n("Edit")
                            display: AbstractButton.IconOnly

                            Component {
                                id: imageEditorPage
                                ImageEditorPage {
                                    imagePath: attachmentPaneLoader.attachmentPath
                                }
                            }
                            onClicked: {
                                let imageEditor = applicationWindow().pageStack.layers.push(imageEditorPage);
                                imageEditor.newPathChanged.connect(function(newPath) {
                                    applicationWindow().pageStack.layers.pop();
                                    attachmentPaneLoader.attachmentPath = newPath;
                                });
                            }
                            ToolTip.text: text
                            ToolTip.visible: hovered
                        }
                        ToolButton {
                            id: cancelAttachmentButton
                            icon.name: "dialog-close"
                            text: i18n("Cancel sending Image")
                            display: AbstractButton.IconOnly
                            onClicked: currentRoom.chatBoxAttachmentPath = "";
                            ToolTip.text: text
                            ToolTip.visible: hovered
                        }
                    }
                    background: null
                }
            }

            background: Rectangle {
                color: Kirigami.Theme.backgroundColor
            }
        }
    }
}
