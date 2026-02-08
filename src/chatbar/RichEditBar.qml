// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat.libneochat as LibNeoChat
import org.kde.neochat.messagecontent as MessageContent

pragma ComponentBehavior: Bound

RowLayout {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property LibNeoChat.NeoChatRoom room

    required property MessageContent.ChatBarMessageContentModel contentModel

    required property real maxAvailableWidth

    readonly property real uncompressedImplicitWidth: boldButton.implicitWidth +
                                                      italicButton.implicitWidth +
                                                      extraTextFormatRow.implicitWidth +
                                                      listRow.implicitWidth +
                                                      styleButton.implicitWidth +
                                                      emojiButton.implicitWidth +
                                                      linkButton.implicitWidth +
                                                      root.spacing * 7 +
                                                      Kirigami.Units.gridUnit

    readonly property real listCompressedImplicitWidth: boldButton.implicitWidth +
                                                        italicButton.implicitWidth +
                                                        extraTextFormatRow.implicitWidth +
                                                        compressedListButton.implicitWidth +
                                                        styleButton.uncompressedWidth +
                                                        emojiButton.implicitWidth +
                                                        linkButton.implicitWidth +
                                                        root.spacing * 7 +
                                                        Kirigami.Units.gridUnit

    readonly property real extraTextCompressedImplicitWidth: boldButton.implicitWidth +
                                                             italicButton.implicitWidth +
                                                             compressedExtraTextFormatButton.implicitWidth +
                                                             compressedListButton.implicitWidth +
                                                             styleButton.uncompressedWidth +
                                                             emojiButton.implicitWidth +
                                                             linkButton.implicitWidth +
                                                             root.spacing * 7 +
                                                             Kirigami.Units.gridUnit

    readonly property ChatButtonHelper chatButtonHelper: ChatButtonHelper {
        textItem: root.contentModel.focusedTextItem
        inQuote: root.contentModel.focusType == LibNeoChat.MessageComponentType.Quote
        hasAttachment: root.contentModel.hasAttachment
    }

    signal clicked

    QQC2.ToolButton {
        id: boldButton
        Shortcut {
            sequence: "Ctrl+B"
            onActivated: boldButton.clicked()
        }
        icon.name: "format-text-bold"
        enabled: root.chatButtonHelper.richFormatEnabled
        text: i18nc("@action:button", "Bold")
        display: QQC2.AbstractButton.IconOnly
        checkable: true
        checked: root.chatButtonHelper.bold
        onClicked: {
            root.chatButtonHelper.setFormat(LibNeoChat.RichFormat.Bold);
            root.clicked()
        }

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
        enabled: root.chatButtonHelper.richFormatEnabled
        text: i18nc("@action:button", "Italic")
        display: QQC2.AbstractButton.IconOnly
        checkable: true
        checked: root.chatButtonHelper.italic
        onClicked: {
            root.chatButtonHelper.setFormat(LibNeoChat.RichFormat.Italic);
            root.clicked()
        }

        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
    RowLayout {
        id: extraTextFormatRow
        visible: root.maxAvailableWidth > root.listCompressedImplicitWidth
        QQC2.ToolButton {
            id: underlineButton
            Shortcut {
                sequence: "Ctrl+U"
                onActivated: underlineButton.clicked()
            }
            icon.name: "format-text-underline"
            enabled: root.chatButtonHelper.richFormatEnabled
            text: i18nc("@action:button", "Underline")
            display: QQC2.AbstractButton.IconOnly
            checkable: true
            checked: root.chatButtonHelper.underline
            onClicked: {
                root.chatButtonHelper.setFormat(LibNeoChat.RichFormat.Underline);
                root.clicked();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            icon.name: "format-text-strikethrough"
            enabled: root.chatButtonHelper.richFormatEnabled
            text: i18nc("@action:button", "Strikethrough")
            display: QQC2.AbstractButton.IconOnly
            checkable: true
            checked: root.chatButtonHelper.strikethrough
            onClicked: {
                root.chatButtonHelper.setFormat(LibNeoChat.RichFormat.Strikethrough);
                root.clicked()
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }
    QQC2.ToolButton {
        id: compressedExtraTextFormatButton
        visible: root.maxAvailableWidth < root.listCompressedImplicitWidth
        icon.name: "dialog-text-and-font"
        enabled: root.chatButtonHelper.richFormatEnabled
        text: i18nc("@action:button", "Format Text")
        display: QQC2.AbstractButton.IconOnly
        checkable: true
        onClicked: {
            let dialog = compressedTextFormatMenu.createObject(compressedExtraTextFormatButton) as QQC2.Menu
            dialog.onClosed.connect(() => {
                compressedExtraTextFormatButton.checked = false;
            });
            dialog.open();
            compressedExtraTextFormatButton.checked = true;
        }

        Component {
            id: compressedTextFormatMenu
            QQC2.Menu {
                y: -implicitHeight

                QQC2.MenuItem {
                    icon.name: "format-text-underline"
                    text: i18nc("@action:button", "Underline")
                    checkable: true
                    checked: root.chatButtonHelper.underline
                    onTriggered: {
                        root.chatButtonHelper.setFormat(LibNeoChat.RichFormat.Underline);
                        root.clicked();
                    }
                }
                QQC2.MenuItem {
                    icon.name: "format-text-strikethrough"
                    text: i18nc("@action:button", "Strikethrough")
                    checkable: true
                    checked: root.chatButtonHelper.strikethrough
                    onTriggered: {
                        root.chatButtonHelper.setFormat(LibNeoChat.RichFormat.Strikethrough);
                        root.clicked();
                    }
                }
            }
        }

        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
    StyleButton {
        id: styleButton
        Layout.minimumWidth: compressed ? -1 : Kirigami.Units.gridUnit * 10 + Kirigami.Units.largeSpacing * 2

        icon.name: "typewriter"
        text: i18nc("@action:button", "Text Style")
        style: root.chatButtonHelper.currentStyle
        inQuote: root.contentModel.focusType == LibNeoChat.MessageComponentType.Quote
        compressed: root.maxAvailableWidth < root.extraTextCompressedImplicitWidth
        enabled: root.chatButtonHelper.styleFormatEnabled
        display: QQC2.AbstractButton.IconOnly
        checkable: true
        checked: styleMenu.visible
        onClicked: {
            if (styleMenu.visible) {
                styleMenu.close();
                return;
            }
            open = true;
            styleMenu.open();
        }

        StylePicker {
            id: styleMenu
            width: styleButton.compressed ? implicitWidth : styleButton.width
            chatContentModel: root.contentModel
            chatButtonHelper: root.chatButtonHelper
            inQuote: root.contentModel.focusType == LibNeoChat.MessageComponentType.Quote

            onClosed: {
                root.clicked()
                styleButton.open = false;
            }
        }
    }
    QQC2.ToolButton {
        id: emojiButton
        visible: !Kirigami.Settings.isMobile
        icon.name: "smiley"
        text: i18n("Emojis & Stickers")
        display: QQC2.AbstractButton.IconOnly
        checkable: true

        onClicked: {
            let dialog = (emojiDialog.createObject(root) as EmojiDialog).open();
        }
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        QQC2.ToolTip.text: text
    }
    QQC2.ToolButton {
        id: linkButton
        enabled: root.chatButtonHelper.richFormatEnabled
        icon.name: "insert-link-symbolic"
        text: root.chatButtonHelper.currentLinkUrl.length > 0 ? i18nc("@action:button", "Edit link") : i18nc("@action:button", "Insert link")
        display: QQC2.AbstractButton.IconOnly
        onClicked: {
            let dialog = linkDialog.createObject(QQC2.Overlay.overlay, {
                linkText: root.chatButtonHelper.currentLinkText,
                linkUrl: root.chatButtonHelper.currentLinkUrl
            })
            dialog.onAccepted.connect(() => {
                root.chatButtonHelper.updateLink(dialog.linkUrl, dialog.linkText)
                root.clicked();
            });
            dialog.open();
        }

        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
    RowLayout {
        id: listRow
        visible: root.maxAvailableWidth > root.uncompressedImplicitWidth
        QQC2.ToolButton {
            icon.name: "format-list-unordered"
            enabled: root.chatButtonHelper.richFormatEnabled
            text: i18nc("@action:button", "Unordered List")
            display: QQC2.AbstractButton.IconOnly
            checkable: true
            checked: root.chatButtonHelper.unorderedList
            onClicked: {
                root.chatButtonHelper.setFormat(LibNeoChat.RichFormat.UnorderedList);
                root.clicked();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            icon.name: "format-list-ordered"
            enabled: root.chatButtonHelper.richFormatEnabled
            text: i18nc("@action:button", "Ordered List")
            display: QQC2.AbstractButton.IconOnly
            checkable: true
            checked: root.chatButtonHelper.orderedList
            onClicked: {
                root.chatButtonHelper.setFormat(LibNeoChat.RichFormat.OrderedList);
                root.clicked();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            id: indentAction
            icon.name: "format-indent-more"
            enabled: root.chatButtonHelper.richFormatEnabled && root.chatButtonHelper.canIndentListMore
            text: i18nc("@action:button", "Increase List Level")
            display: QQC2.AbstractButton.IconOnly
            onClicked: {
                root.chatButtonHelper.indentListMore();
                root.clicked();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            id: dedentAction
            icon.name: "format-indent-less"
            enabled: root.chatButtonHelper.richFormatEnabled && root.chatButtonHelper.canIndentListLess
            text: i18nc("@action:button", "Decrease List Level")
            display: QQC2.AbstractButton.IconOnly
            onClicked: {
                root.chatButtonHelper.indentListLess();
                root.clicked();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }
    QQC2.ToolButton {
        id: compressedListButton
        enabled: root.chatButtonHelper.richFormatEnabled
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
                onTriggered: {
                    root.chatButtonHelper.setFormat(LibNeoChat.RichFormat.UnorderedList);
                    root.clicked();
                }
            }
            QQC2.MenuItem {
                icon.name: "format-list-ordered"
                text: i18nc("@action:button", "Ordered List")
                onTriggered: {
                    root.chatButtonHelper.setFormat(LibNeoChat.RichFormat.OrderedList);
                    root.clicked();
                }
            }
            QQC2.MenuItem {
                icon.name: "format-indent-more"
                text: i18nc("@action:button", "Increase List Level")
                enabled: root.chatButtonHelper.canIndentListMore
                onTriggered: {
                    root.chatButtonHelper.indentListMore();
                    root.clicked();
                }
            }
            QQC2.MenuItem {
                icon.name: "format-indent-less"
                text: i18nc("@action:button", "Decrease List Level")
                enabled: root.chatButtonHelper.canIndentListLess
                onTriggered: {
                    root.chatButtonHelper.indentListLess();
                    root.clicked();
                }
            }
        }

        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    Component {
        id: linkDialog
        LinkDialog {}
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
                root.chatButtonHelper.insertText(emoji);
                close();
            }
            onClosed: if (emojiButton.checked) {
                emojiButton.checked = false;
            }
        }
    }
}
