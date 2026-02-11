// SPDX-FileCopyrightText: 2026 Azhar Momin <azhar.momin@kdemail.net>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.InlineMessage {
    id: root

    /**
     * @brief The NeoChatRoom the selected messages belong to.
     */
    required property NeoChatRoom room

    showCloseButton: false
    visible: root.room?.selectedMessageCount > 0
    position: Kirigami.InlineMessage.Position.Header
    type: Kirigami.MessageType.Positive
    icon.name: "edit-select-all-symbolic"

    text: i18nc("@info", "Selected Messages: %1", root.room?.selectedMessageCount)

    actions: [
        Kirigami.Action {
            text: i18nc("@action:button", "Copy Conversation")
            icon.name: "edit-copy"
            onTriggered: {
                Clipboard.saveText(root.room.getFormattedSelectedMessages())
                showPassiveNotification(i18nc("@info", "Conversation copied to clipboard"));
            }
        },
        Kirigami.Action {
            text: i18nc("@action:button", "Delete Messages")
            icon.name: "trash-empty-symbolic"
            icon.color: Kirigami.Theme.negativeTextColor
            enabled: root.room?.canDeleteSelectedMessages
            onTriggered: {
                let dialog = pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
                    title: i18nc("@title:dialog", "Remove Messages"),
                    placeholder: i18nc("@info:placeholder", "Optionally give a reason for removing these messages"),
                    actionText: i18nc("@action:button 'Remove' as in 'Remove these messages'", "Remove"),
                    icon: "delete",
                    reporting: false,
                    connection: root.room.connection,
                }, {
                    title: i18nc("@title:dialog", "Remove Messages"),
                    width: Kirigami.Units.gridUnit * 25
                }) as ReasonDialog;
                dialog.accepted.connect(reason => {
                    root.room.deleteSelectedMessages(reason);
                });
            }
        },
        Kirigami.Action {
            icon.name: "dialog-close"
            icon.color: Kirigami.Theme.negativeTextColor
            onTriggered: root.room.clearSelectedMessages()
        }
    ]
}
