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

    property LibNeoChat.ChatBarCache chatBarCache

    required property MessageContent.ChatBarMessageContentModel contentModel

    required property real maxAvailableWidth

    readonly property real overflowWidth: Kirigami.Units.gridUnit * 30

    function openLocationChooser(): void {
        Qt.createComponent('org.kde.neochat.chatbar', 'LocationChooser').createObject(QQC2.ApplicationWindow.overlay, {
            room: root.room
        }).open();
    }

    function openNewPollDialog(): void {
        Qt.createComponent('org.kde.neochat.chatbar', 'NewPollDialog').createObject(QQC2.Overlay.overlay, {
            room: root.room
        }).open();
    }

    function addAttachment(): void {
        if (!root.contentModel.hasRichFormatting) {
            if (LibNeoChat.Clipboard.hasImage) {
                attachDialog();
            } else {
                fileDialog();
            }
            return;
        }

        let warningDialog = Qt.createComponent('org.kde.kirigami', 'PromptDialog').createObject(QQC2.Overlay.overlay, {
            dialogType: Kirigami.PromptDialog.Warning,
            title: attachmentButton.text,
            subtitle: i18nc("@Warning: that any rich text in the chat bar will be switched for the plain text equivalent.", "Attachments can only have plain text captions, all rich formatting will be removed"),
            standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        });
        warningDialog.onAccepted.connect(() => {
            if (LibNeoChat.Clipboard.hasImage) {
                attachmentButton.attachDialog();
            } else {
                attachmentButton.fileDialog();
            }
        });
        warningDialog.open();
    }

    function attachDialog(): void {
        let dialog = Qt.createComponent('org.kde.neochat.chatbar', 'AttachDialog').createObject(QQC2.Overlay.overlay) as AttachDialog;
        dialog.anchors.centerIn = QQC2.Overlay.overlay;
        dialog.chosen.connect(path => root.contentModel.addAttachment(path));
        dialog.open();
    }

    function fileDialog(): void {
        let dialog = Qt.createComponent('org.kde.neochat.libneochat', 'OpenFileDialog').createObject(QQC2.Overlay.overlay, {
            parentWindow: Window.window,
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
        });
        dialog.chosen.connect(path => root.contentModel.addAttachment(path));
        dialog.open();
    }

    function openVoiceDialog(): void {
        let dialog = Qt.createComponent('org.kde.neochat.chatbar', 'VoiceMessageDialog').createObject(root, {
            room: root.currentRoom
        }) as VoiceMessageDialog;
        dialog.open();
    }

    Kirigami.Separator {
        Layout.fillHeight: true
    }
    QQC2.ToolButton {
        id: compressedExtraSendButton
        property QQC2.Menu overflowMenu

        visible: root.maxAvailableWidth < root.overflowWidth && (root.contentModel?.type ?? true) === LibNeoChat.ChatBarType.Room
        icon.name: "list-add-symbolic"
        text: i18nc("@action:button", "Add to message")
        display: QQC2.AbstractButton.IconOnly
        checkable: true
        checked: overflowMenu !== null

        Accessible.role: Accessible.ButtonMenu

        onClicked: {
            if (!checked) {
                if (overflowMenu) {
                    overflowMenu.close();
                }
                return;
            }

            overflowMenu = compressedExtraSendMenu.createObject(compressedExtraSendButton)
            overflowMenu.onClosed.connect(() => {
                overflowMenu = null;
            });
            overflowMenu.open();
        }

        Component {
            id: compressedExtraSendMenu
            QQC2.Menu {
                y: -implicitHeight

                QQC2.MenuItem {
                    visible: !root.contentModel.hasAttachment
                    icon.name: "mail-attachment"
                    text: i18nc("@action:button", "Attach an image or file")
                    onTriggered: root.addAttachment()
                }
                QQC2.MenuItem {
                    icon.name: "globe"
                    text: i18nc("@action:button", "Send a Location")
                    onTriggered: root.openLocationChooser()
                }
                QQC2.MenuItem {
                    icon.name: "amarok_playcount"
                    text: i18nc("@action:button", "Create a Poll")
                    onTriggered: root.openNewPollDialog();
                }
                QQC2.MenuItem {
                    icon.name: "microphone"
                    text: i18nc("@action:button", "Send a Voice Message")
                    onTriggered: root.openVoiceDialog();
                }
            }
        }

        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
    QQC2.ToolButton {
        id: attachmentButton
        visible: !root.contentModel.hasAttachment &&
                 ((root.contentModel?.type ?? true) === LibNeoChat.ChatBarType.Room || (root.contentModel?.type ?? true) === LibNeoChat.ChatBarType.Thread) &&
                 root.maxAvailableWidth >= root.overflowWidth
        icon.name: "mail-attachment"
        text: i18nc("@action:button", "Attach an image or file")
        display: QQC2.AbstractButton.IconOnly

        onClicked: root.addAttachment()

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        QQC2.ToolTip.text: text
    }
    QQC2.ToolButton {
        id: mapButton
        visible: (root.contentModel?.type ?? true) === LibNeoChat.ChatBarType.Room && root.maxAvailableWidth >= root.overflowWidth
        icon.name: "globe"
        text: i18nc("@action:button", "Send a Location")
        display: QQC2.AbstractButton.IconOnly

        onClicked: root.openLocationChooser();
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        QQC2.ToolTip.text: text
    }
    QQC2.ToolButton {
        id: pollButton
        visible: (root.contentModel?.type ?? true) === LibNeoChat.ChatBarType.Room && root.maxAvailableWidth >= root.overflowWidth
        icon.name: "amarok_playcount"
        text: i18nc("@action:button", "Create a Poll")
        display: QQC2.AbstractButton.IconOnly

        onClicked: root.openNewPollDialog();
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        QQC2.ToolTip.text: text
    }
    QQC2.ToolButton {
        visible: (root.contentModel?.type ?? true) === LibNeoChat.ChatBarType.Room && root.maxAvailableWidth >= root.overflowWidth
        icon.name: "microphone"
        text: i18nc("@action:button", "Send a Voice Message")
        display: QQC2.AbstractButton.IconOnly
        onClicked: root.openVoiceDialog();
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        QQC2.ToolTip.text: text
    }
    QQC2.ToolButton {
        id: sendButton
        icon.name: "document-send"
        text: i18nc("@action:button", "Send message")
        display: QQC2.AbstractButton.IconOnly
        enabled: root.contentModel.hasAnyContent

        onClicked: root.contentModel.postMessage();
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        QQC2.ToolTip.text: text
    }
    QQC2.ToolButton {
        id: cancelButton
        visible: (root.contentModel?.type ?? true) === LibNeoChat.ChatBarType.Edit
        display: QQC2.AbstractButton.IconOnly
        text: i18nc("@action:button", "Cancel")
        icon.name: "dialog-close"
        onClicked: root.room.cacheForType(contentModel.type).clearRelations()

        Kirigami.Action {
            shortcut: "Escape"
            onTriggered: cancelButton.clicked()
        }

        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
}
