// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat
import org.kde.neochat.config

/**
 * @brief The menu for normal messages.
 *
 * This component just overloads the actions and nested actions of the base menu
 * to what is required for a message item.
 *
 * @sa DelegateContextMenu
 */
DelegateContextMenu {
    id: root

    /**
     * @brief The delegate type of the message.
     */
    required property int messageComponentType

    /**
     * @brief The display text of the message as rich text.
     */
    required property string htmlText

    actions: [
        Kirigami.Action {
            text: i18n("Edit")
            icon.name: "document-edit"
            onTriggered: {
                currentRoom.editCache.editId = eventId;
                currentRoom.mainCache.replyId = "";
            }
            visible: author.isLocalUser && (root.messageComponentType === MessageComponentType.Emote || root.messageComponentType === MessageComponentType.Message)
        },
        DelegateContextMenu.ReplyMessageAction {},
        Kirigami.Action {
            text: i18nc("@action:inmenu As in 'Forward this message'", "Forward")
            icon.name: "mail-forward-symbolic"
            onTriggered: {
                let page = applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ChooseRoomDialog.qml'), {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Forward Message"),
                    width: Kirigami.Units.gridUnit * 25
                });
                page.chosen.connect(function (targetRoomId) {
                    root.connection.room(targetRoomId).postHtmlMessage(root.plainText, root.htmlText.length > 0 ? root.htmlText : root.plainText);
                    page.closeDialog();
                });
            }
        },
        DelegateContextMenu.RemoveMessageAction {},
        Kirigami.Action {
            text: i18n("Copy")
            icon.name: "edit-copy"
            onTriggered: Clipboard.saveText(root.selectedText.length > 0 ? root.selectedText : root.plainText)
        },
        DelegateContextMenu.ReportMessageAction {},
        DelegateContextMenu.ViewSourceAction {},
        Kirigami.Action {
            text: i18n("Copy Link")
            icon.name: "edit-copy"
            onTriggered: {
                Clipboard.saveText("https://matrix.to/#/" + currentRoom.id + "/" + root.eventId);
            }
        }
    ]
}
