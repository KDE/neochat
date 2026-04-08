// SPDX-FileCopyrightText: 2026 Azhar Momin <azhar.momin@kdemail.net>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.timeline

Kirigami.InlineMessage {
    id: root

    /**
     * @brief The message model for the selected messages.
     */
    required property MessageModel messageModel

    showCloseButton: false
    visible: root.messageModel?.selectedMessageCount > 0
    position: Kirigami.InlineMessage.Position.Header
    type: Kirigami.MessageType.Positive
    icon.name: "edit-select-all-symbolic"

    text: i18nc("@info", "Selected Messages: %1", root.messageModel?.selectedMessageCount)

    actions: [
        Kirigami.Action {
            text: i18nc("@action:button", "Copy Conversation")
            icon.name: "edit-copy"
            onTriggered: {
                Clipboard.saveText(root.messageModel.getFormattedSelectedMessages())
                let window = (root.QQC2.ApplicationWindow.window as Main)
                window.showPassiveNotification(i18nc("@info", "Conversation copied to clipboard"));
            }
        },
        Kirigami.Action {
            text: i18nc("@action:button", "Delete Messages")
            icon.name: "trash-empty-symbolic"
            icon.color: Kirigami.Theme.negativeTextColor
            enabled: root.messageModel?.canDeleteSelectedMessages
            onTriggered: {
                let dialog = (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
                    title: i18nc("@title:dialog", "Remove Messages"),
                    placeholder: i18nc("@info:placeholder", "Optionally give a reason for removing these messages"),
                    actionText: i18nc("@action:button 'Remove' as in 'Remove these messages'", "Remove"),
                    icon: "delete",
                    reporting: false,
                }, {
                    title: i18nc("@title:dialog", "Remove Messages"),
                    width: Kirigami.Units.gridUnit * 25
                }) as ReasonDialog;
                dialog.accepted.connect(reason => {
                    root.messageModel.deleteSelectedMessages(reason);
                });
            }
        },
        Kirigami.Action {
            icon.name: "dialog-close"
            icon.color: Kirigami.Theme.negativeTextColor
            onTriggered: root.messageModel.clearSelectedMessages()
        }
    ]
}
