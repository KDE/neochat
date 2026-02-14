// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat

import Quotient

Kirigami.PromptDialog {
    id: root

    required property NeoChatRoom room

    title: root.room.isSpace ? i18nc("@title:dialog", "Confirm Leaving Space") : i18nc("@title:dialog", "Confirm Leaving Room")
    subtitle: {
        if (root.room) {
            let message = xi18nc("@info Do you really want to leave <room name>?", "Do you really want to leave %1?", root.room.displayNameForHtml)

            // List any possible side-effects the user needs to be made aware of.
            if (root.room.historyVisibility !== "world_readable" && root.room.historyVisibility !== "shared") {
                message += xi18nc("@info", "<br><strong>This room's history is limited to when you rejoin the room.</strong>")
            }

            if (root.room.joinRule === JoinRule.JoinRule.Invite) {
                message += xi18nc("@info", "<br><strong>This room can only be rejoined with an invite.</strong>");
            }

            return message;
        }
        return "";
    }
    dialogType: Kirigami.PromptDialog.Warning

    onRejected: {
        root.close();
    }

    footer: QQC2.DialogButtonBox {
        standardButtons: QQC2.Dialog.Cancel

        QQC2.Button {
            text: i18nc("@action:button", "Leave Room")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            icon.name: "arrow-left-symbolic"
            //onClicked: root.room.forget();
        }
    }
}
