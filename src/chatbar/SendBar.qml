// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat.libneochat as LibNeoChat
import org.kde.neochat.messagecontent as MessageContent

RowLayout {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property LibNeoChat.NeoChatRoom room

    property LibNeoChat.ChatBarCache chatBarCache

    required property MessageContent.ChatBarMessageContentModel contentModel

    Kirigami.Separator {
        Layout.fillHeight: true
    }
    QQC2.ToolButton {
        id: attachmentButton

        property bool isBusy: root.room && root.room.hasFileUploading

        visible: !root.contentModel.hasAttachment && (root.contentModel?.type ?? true) === LibNeoChat.ChatBarType.Room
        icon.name: "mail-attachment"
        text: i18n("Attach an image or file")
        display: QQC2.AbstractButton.IconOnly

        onClicked: {
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
                title: i18n("Attach an image or file?"),
                subtitle: i18n("Attachments can only have plain text captions, all rich formatting will be removed"),
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

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        QQC2.ToolTip.text: text
    }
    QQC2.ToolButton {
        id: mapButton
        visible: (root.contentModel?.type ?? true) === LibNeoChat.ChatBarType.Room
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
        visible: (root.contentModel?.type ?? true) === LibNeoChat.ChatBarType.Room
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
    QQC2.ToolButton {
        id: sendButton

        property bool isBusy: false

        icon.name: "document-send"
        text: i18n("Send message")
        display: QQC2.AbstractButton.IconOnly

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
