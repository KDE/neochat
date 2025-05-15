// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtTextToSpeech
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.neochat

import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

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

    Kirigami.Action {
        text: i18n("Edit")
        icon.name: "document-edit"
        onTriggered: {
            currentRoom.editCache.editId = eventId;
            currentRoom.mainCache.replyId = "";
            currentRoom.mainCache.threadId = "";
        }
        visible: root.author.isLocalMember && root.messageComponentType === MessageComponentType.Text
    }

    DelegateContextMenu.ReplyMessageAction {}

    DelegateContextMenu.ReplyThreadMessageAction {}

    QQC2.Action {
        text: i18nc("@action:inmenu As in 'Forward this message'", "Forward…")
        icon.name: "mail-forward-symbolic"
        onTriggered: {
            let page = applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ChooseRoomDialog'), {
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
    }
    Kirigami.Action {
        separator: true
    }
    DelegateContextMenu.RemoveMessageAction {}
    Kirigami.Action {
        text: i18nc("@action:inmenu", "Copy Link Address")
        icon.name: "edit-copy"
        visible: root.hoveredLink.length > 0
        onTriggered: Clipboard.saveText(root.hoveredLink)
    }
    QQC2.Action {
        text: i18nc("@action:inmenu", "Copy Text")
        icon.name: "edit-copy"
        onTriggered: Clipboard.saveText(root.selectedText.length > 0 ? root.selectedText : root.plainText)
    }
    QQC2.Action {
        text: i18nc("@action:inmenu", "Copy Message Link")
        icon.name: "link-symbolic"
        onTriggered: {
            Clipboard.saveText("https://matrix.to/#/" + currentRoom.id + "/" + root.eventId);
        }
    }
    QQC2.Action {
        text: i18nc("@action:inmenu", "Read Text Aloud")
        icon.name: "audio-speakers-symbolic"
        onTriggered: {
            TextToSpeechHelper.speak(i18nc("@info text-to-speech %1 is author %2 is message text", "%1 said %2", root.author.displayName, root.plainText))
        }
    }
    Kirigami.Action {
        separator: true
    }
    DelegateContextMenu.PinMessageAction {}
    DelegateContextMenu.ReportMessageAction {}
    Kirigami.Action {
        separator: true
        visible: viewSourceAction.visible
    }
    DelegateContextMenu.ViewSourceAction {
        id: viewSourceAction
    }

    Kirigami.Action {
        separator: true
        visible: webShortcutModel.enabled
    }

    Kirigami.Action {
        id: webShortcutModelAction

        text: i18n("Search for '%1'", webShortcutModel.trunkatedSearchText)
        icon.name: "search-symbolic"
        visible: webShortcutModel.enabled

        readonly property Instantiator instantiator: Instantiator {
            model: WebShortcutModel {
                id: webShortcutModel
                selectedText: root.selectedText.length > 0 ? root.selectedText : root.plainText
                onOpenUrl: url => RoomManager.resolveResource(url.toString())
            }
            delegate: QQC2.Action {
                text: model.display
                icon.name: model.decoration
                onTriggered: webShortcutModel.trigger(model.edit)
            }
            onObjectAdded: (index, object) => webShortcutModelAction.children.push(object)
        }
    }

    Kirigami.Action {
        text: i18n("Configure Web Shortcuts…")
        icon.name: "configure"
        visible: !Controller.isFlatpak && webShortcutModel.enabled
        onTriggered: webShortcutModel.configureWebShortcuts()
    }
}
