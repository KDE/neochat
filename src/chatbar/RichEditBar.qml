// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat.libneochat as LibNeoChat

QQC2.ToolBar {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property LibNeoChat.NeoChatRoom room

    property LibNeoChat.ChatBarCache chatBarCache

    required property LibNeoChat.ChatDocumentHandler documentHandler

    required property real maxAvailableWidth

    readonly property real uncompressedImplicitWidth: textFormatRow.implicitWidth +
                                                      listRow.implicitWidth +
                                                      styleButton.implicitWidth +
                                                      emojiButton.implicitWidth +
                                                      linkButton.implicitWidth +
                                                      sendRow.implicitWidth +
                                                      sendButton.implicitWidth +
                                                      buttonRow.spacing * 9 +
                                                      3

    readonly property real listCompressedImplicitWidth: textFormatRow.implicitWidth +
                                                        compressedListButton.implicitWidth +
                                                        styleButton.implicitWidth +
                                                        emojiButton.implicitWidth +
                                                        linkButton.implicitWidth +
                                                        sendRow.implicitWidth +
                                                        sendButton.implicitWidth +
                                                        buttonRow.spacing * 9 +
                                                        3

    readonly property real textFormatCompressedImplicitWidth: compressedTextFormatButton.implicitWidth +
                                                              compressedListButton.implicitWidth +
                                                              styleButton.implicitWidth +
                                                              emojiButton.implicitWidth +
                                                              linkButton.implicitWidth +
                                                              sendRow.implicitWidth +
                                                              sendButton.implicitWidth +
                                                              buttonRow.spacing * 9 +
                                                              3

    signal requestPostMessage

    RowLayout {
        id: buttonRow
        RowLayout {
            id: textFormatRow
            visible: root.maxAvailableWidth > root.listCompressedImplicitWidth
            QQC2.ToolButton {
                id: boldButton
                Shortcut {
                    sequence: "Ctrl+B"
                    onActivated: boldButton.clicked()
                }
                icon.name: "format-text-bold"
                text: i18nc("@action:button", "Bold")
                display: QQC2.AbstractButton.IconOnly
                checkable: true
                checked: root.documentHandler.bold
                onClicked: root.documentHandler.bold = checked;

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            QQC2.ToolButton {
                id: italicButton
                Shortcut {
                    sequence: "Ctrl+I"
                    onActivated: italicButton.clicked()
                }
                icon.name: "format-text-italic"
                text: i18nc("@action:button", "Italic")
                display: QQC2.AbstractButton.IconOnly
                checkable: true
                checked: root.documentHandler.italic
                onClicked: root.documentHandler.italic = checked;

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            QQC2.ToolButton {
                id: underlineButton
                Shortcut {
                    sequence: "Ctrl+U"
                    onActivated: underlineButton.clicked()
                }
                icon.name: "format-text-underline"
                text: i18nc("@action:button", "Underline")
                display: QQC2.AbstractButton.IconOnly
                checkable: true
                checked: root.documentHandler.underline
                onClicked: root.documentHandler.underline = checked;

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            QQC2.ToolButton {
                icon.name: "format-text-strikethrough"
                text: i18nc("@action:button", "Strikethrough")
                display: QQC2.AbstractButton.IconOnly
                checkable: true
                checked: root.documentHandler.strikethrough
                onClicked: root.documentHandler.strikethrough = checked;

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
        QQC2.ToolButton {
            id: compressedTextFormatButton
            visible: root.maxAvailableWidth < root.listCompressedImplicitWidth
            icon.name: "dialog-text-and-font"
            text: i18nc("@action:button", "Format Text")
            display: QQC2.AbstractButton.IconOnly
            checkable: true
            checked: compressedTextFormatMenu.visible
            onClicked: {
                compressedTextFormatMenu.open()
            }

            QQC2.Menu {
                id: compressedTextFormatMenu
                y: -implicitHeight

                QQC2.MenuItem {
                    icon.name: "format-text-bold"
                    text: i18nc("@action:button", "Bold")
                    checkable: true
                    checked: root.documentHandler.bold
                    onTriggered: root.documentHandler.bold = checked;
                }
                QQC2.MenuItem {
                    icon.name: "format-text-italic"
                    text: i18nc("@action:button", "Italic")
                    checkable: true
                    checked: root.documentHandler.italic
                    onTriggered: root.documentHandler.italic = checked;
                }
                QQC2.MenuItem {
                    icon.name: "format-text-underline"
                    text: i18nc("@action:button", "Underline")
                    checkable: true
                    checked: root.documentHandler.underline
                    onTriggered: root.documentHandler.underline = checked;
                }
                QQC2.MenuItem {
                    icon.name: "format-text-strikethrough"
                    text: i18nc("@action:button", "Strikethrough")
                    checkable: true
                    checked: root.documentHandler.strikethrough
                    onTriggered: root.documentHandler.strikethrough = checked;
                }
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        Kirigami.Separator {
            Layout.fillHeight: true
            Layout.margins: 0
        }
        RowLayout {
            id: listRow
            visible: root.maxAvailableWidth > root.uncompressedImplicitWidth
            QQC2.ToolButton {
                icon.name: "format-list-unordered"
                text: i18nc("@action:button", "Unordered List")
                display: QQC2.AbstractButton.IconOnly
                checkable: true
                checked: root.documentHandler.currentListStyle === 1
                onClicked: {
                    root.documentHandler.setListStyle(root.documentHandler.currentListStyle === 1 ? 0 : 1)
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            QQC2.ToolButton {
                icon.name: "format-list-ordered"
                text: i18nc("@action:button", "Ordered List")
                display: QQC2.AbstractButton.IconOnly
                checkable: true
                checked: root.documentHandler.currentListStyle === 4
                onClicked: root.documentHandler.setListStyle(root.documentHandler.currentListStyle === 4 ? 0 : 4)

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            QQC2.ToolButton {
                id: indentAction
                icon.name: "format-indent-more"
                text: i18nc("@action:button", "Increase List Level")
                display: QQC2.AbstractButton.IconOnly
                onClicked: {
                    root.documentHandler.indentListMore();
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
            QQC2.ToolButton {
                id: dedentAction
                icon.name: "format-indent-less"
                text: i18nc("@action:button", "Decrease List Level")
                display: QQC2.AbstractButton.IconOnly
                onClicked: {
                    root.documentHandler.indentListLess();
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
        QQC2.ToolButton {
            id: compressedListButton
            visible: root.maxAvailableWidth < root.uncompressedImplicitWidth
            icon.name: "format-list-unordered"
            text: i18nc("@action:button", "List Style")
            display: QQC2.AbstractButton.IconOnly
            checkable: true
            checked: compressedListMenu.visible
            onClicked: {
                compressedListMenu.open()
            }

            QQC2.Menu {
                id: compressedListMenu
                y: -implicitHeight

                QQC2.MenuItem {
                    icon.name: "format-list-unordered"
                    text: i18nc("@action:button", "Unordered List")
                    onTriggered: root.documentHandler.setListStyle(root.documentHandler.currentListStyle === 1 ? 0 : 1);
                }
                QQC2.MenuItem {
                    icon.name: "format-list-ordered"
                    text: i18nc("@action:button", "Ordered List")
                    onTriggered: root.documentHandler.setListStyle(root.documentHandler.currentListStyle === 4 ? 0 : 4);
                }
                QQC2.MenuItem {
                    icon.name: "format-indent-more"
                    text: i18nc("@action:button", "Increase List Level")
                    onTriggered: root.documentHandler.indentListMore();
                }
                QQC2.MenuItem {
                    icon.name: "format-indent-less"
                    text: i18nc("@action:button", "Decrease List Level")
                    onTriggered: root.documentHandler.indentListLess();
                }
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            id: styleButton
            icon.name: "typewriter"
            text: i18nc("@action:button", "Text Style")
            display: QQC2.AbstractButton.IconOnly
            checkable: true
            checked: styleMenu.visible
            onClicked: {
                styleMenu.open()
            }

            QQC2.Menu {
                id: styleMenu
                y: -implicitHeight

                QQC2.MenuItem {
                    text: i18nc("@item:inmenu no heading", "Paragraph")
                    onTriggered: root.documentHandler.setHeadingLevel(0);
                }
                QQC2.MenuItem {
                    text: i18nc("@item:inmenu heading level 1 (largest)", "Heading 1")
                    onTriggered: root.documentHandler.setHeadingLevel(1);
                }
                QQC2.MenuItem {
                    text: i18nc("@item:inmenu heading level 2", "Heading 2")
                    onTriggered: root.documentHandler.setHeadingLevel(2);
                }
                QQC2.MenuItem {
                    text: i18nc("@item:inmenu heading level 3", "Heading 3")
                    onTriggered: root.documentHandler.setHeadingLevel(3);
                }
                QQC2.MenuItem {
                    text: i18nc("@item:inmenu heading level 4", "Heading 4")
                    onTriggered: root.documentHandler.setHeadingLevel(4);
                }
                QQC2.MenuItem {
                    text: i18nc("@item:inmenu heading level 5", "Heading 5")
                    onTriggered: root.documentHandler.setHeadingLevel(5);
                }
                QQC2.MenuItem {
                    text: i18nc("@item:inmenu heading level 6 (smallest)", "Heading 6")
                    onTriggered: root.documentHandler.setHeadingLevel(6);
                }
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        Kirigami.Separator {
            Layout.fillHeight: true
            Layout.margins: 0
        }
        QQC2.ToolButton {
            id: emojiButton

            property bool isBusy: false

            visible: !Kirigami.Settings.isMobile
            icon.name: "smiley"
            text: i18n("Emojis & Stickers")
            display: QQC2.AbstractButton.IconOnly
            checkable: true

            onClicked: {
                let dialog = emojiDialog.createObject(root).open();
            }
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.text: text
        }
        QQC2.ToolButton {
            id: linkButton
            icon.name: "insert-link-symbolic"
            text: i18nc("@action:button", "Insert link")
            display: QQC2.AbstractButton.IconOnly
            onClicked: {
                let dialog = linkDialog.createObject(QQC2.Overlay.overlay, {
                    linkText: root.documentHandler.currentLinkText(),
                    linkUrl: root.documentHandler.currentLinkUrl()
                })
                dialog.onAccepted.connect(() => { documentHandler.updateLink(dialog.linkUrl, dialog.linkText) });
                dialog.open();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        Kirigami.Separator {
            Layout.fillHeight: true
            Layout.margins: 0
        }
        RowLayout {
            id: sendRow
            visible: root.maxAvailableWidth > root.textFormatCompressedImplicitWidth
            QQC2.ToolButton {
                id: attachmentButton

                property bool isBusy: root.room && root.room.hasFileUploading

                visible: root.chatBarCache.attachmentPath.length === 0
                icon.name: "mail-attachment"
                text: i18n("Attach an image or file")
                display: QQC2.AbstractButton.IconOnly

                onClicked: {
                    let dialog = (LibNeoChat.Clipboard.hasImage ? attachDialog : openFileDialog).createObject(QQC2.Overlay.overlay);
                    dialog.chosen.connect(path => root.chatBarCache.attachmentPath = path);
                    dialog.open();
                }
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.text: text
            }
            QQC2.ToolButton {
                id: mapButton
                icon.name: "globe"
                property bool isBusy: false
                text: i18n("Send a Location")
                display: QQC2.AbstractButton.IconOnly

                onClicked: {
                    locationChooser.createObject(QQC2.ApplicationWindow.overlay, {
                        room: root.room
                    }).open();
                }
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.text: text
            }
            QQC2.ToolButton {
                id: pollButton
                icon.name: "amarok_playcount"
                property bool isBusy: false
                text: i18nc("@action:button", "Create a Poll")
                display: QQC2.AbstractButton.IconOnly

                onClicked: {
                    newPollDialog.createObject(QQC2.Overlay.overlay, {
                        room: root.room
                    }).open();
                }
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.text: text
            }
        }
        QQC2.ToolButton {
            id: compressedSendButton
            visible: root.maxAvailableWidth < root.textFormatCompressedImplicitWidth
            icon.name: "overflow-menu"
            text: i18nc("@action:button", "Send Other")
            display: QQC2.AbstractButton.IconOnly
            checkable: true
            checked: compressedSendMenu.visible
            onClicked: {
                compressedSendMenu.open()
            }

            QQC2.Menu {
                id: compressedSendMenu
                y: -implicitHeight

                QQC2.MenuItem {
                    visible: root.chatBarCache.attachmentPath.length === 0
                    icon.name: "mail-attachment"
                    text: i18n("Attach an image or file")
                    onTriggered: {
                        let dialog = (LibNeoChat.Clipboard.hasImage ? attachDialog : openFileDialog).createObject(QQC2.Overlay.overlay);
                        dialog.chosen.connect(path => root.chatBarCache.attachmentPath = path);
                        dialog.open();
                    }
                }
                QQC2.MenuItem {
                    icon.name: "globe"
                    text: i18n("Send a Location")
                    onTriggered: {
                        locationChooser.createObject(QQC2.ApplicationWindow.overlay, {
                            room: root.room
                        }).open();
                    }
                }
                QQC2.MenuItem {
                    icon.name: "amarok_playcount"
                    text: i18nc("@action:button", "Create a Poll")
                    onTriggered: {
                        newPollDialog.createObject(QQC2.Overlay.overlay, {
                            room: root.room
                        }).open();
                    }
                }
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            id: sendButton

            property bool isBusy: false

            icon.name: "document-send"
            text: i18n("Send message")
            display: QQC2.AbstractButton.IconOnly
            checkable: true

            onClicked: root.requestPostMessage()
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.text: text
        }
    }

    background: Kirigami.ShadowedRectangle {
        color: Kirigami.Theme.backgroundColor
        radius: 5

        shadow {
            size: 15
            yOffset: 3
            color: Qt.rgba(0, 0, 0, 0.2)
        }

        border {
            color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.2)
            width: 1
        }

        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
    }

    Component {
        id: linkDialog
        LinkDialog {}
    }

    Component {
        id: attachDialog
        AttachDialog {
            anchors.centerIn: parent
        }
    }

    Component {
        id: openFileDialog
        LibNeoChat.OpenFileDialog {
            parentWindow: Window.window
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
        }
    }

    Component {
        id: emojiDialog
        EmojiDialog {
            x: root.width - width
            y: -implicitHeight

            modal: false
            includeCustom: true
            closeOnChosen: false

            currentRoom: root.room

            onChosen: emoji => {
                root.documentHandler.insertText(emoji);
                close();
            }
            onClosed: if (emojiButton.checked) {
                emojiButton.checked = false;
            }
        }
    }

    Component {
        id: locationChooser
        LocationChooser {}
    }

    Component {
        id: newPollDialog
        NewPollDialog {}
    }
}
