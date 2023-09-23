// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.Page {
    id: root

    property NeoChatRoom room
    property string eventId

    property string userId: ""

    title: userId.length > 0 ? i18n("Remove Messages") : i18n("Remove Message")

    QQC2.TextArea {
        id: reason
        placeholderText: userId.length > 0 ? i18n("Reason for removing this user's recent messages") : i18n("Reason for removing this message")
        anchors.fill: parent
        wrapMode: TextEdit.Wrap
    }

    footer: QQC2.ToolBar {
        QQC2.DialogButtonBox {
            anchors.fill: parent
            Item {
                Layout.fillWidth: true
            }
            QQC2.Button {
                text: i18nc("@action:button 'Remove' as in 'Remove this message'", "Remove")
                icon.name: "delete"
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
                onClicked: {
                    if (root.userId.length > 0) {
                        root.room.deleteMessagesByUser(root.userId, reason.text)
                    } else {
                        root.room.redactEvent(root.eventId, reason.text);
                    }
                    root.closeDialog()
                }
            }
            QQC2.Button {
                text: i18nc("@action", "Cancel")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
                onClicked: root.closeDialog()
            }
        }
    }
}
