/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import org.kde.kirigami 2.14 as Kirigami
import NeoChat.Dialog 1.0
import NeoChat.Menu 1.0

MessageDelegateContextMenu {
    id: root

    signal downloadAndOpen()
    signal saveFileAs()

    property list<Kirigami.Action> actions: [
        Kirigami.Action {
            text: i18n("Open Externally")
            icon.name: "document-open"
            onTriggered: downloadAndOpen()
        },
        Kirigami.Action {
            text: i18n("Save As")
            icon.name: "document-save"
            onTriggered: saveFileAs()
        },
        Kirigami.Action {
            text: i18n("Reply")
            icon.name: "mail-replied-symbolic"
            onTriggered: reply(author, message)
        },
        Kirigami.Action {
            visible: author.id === currentRoom.localUser.id || currentRoom.canSendState("redact")
            text: i18n("Remove")
            icon.name: "edit-delete-remove"
            icon.color: "red"
            onTriggered: remove()
        },
        Kirigami.Action {
            text: i18n("View Source")
            icon.name: "code-context"
            onTriggered: viewSource()
        }
    ]
}
