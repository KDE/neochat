// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0

Kirigami.Page {
    id: deleteSheet

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
                    if (deleteSheet.userId.length > 0) {
                        deleteSheet.room.deleteMessagesByUser(deleteSheet.userId, reason.text)
                    } else {
                        deleteSheet.room.redactEvent(deleteSheet.eventId, reason.text);
                    }
                    deleteSheet.closeDialog()
                }
            }
            QQC2.Button {
                text: i18nc("@action", "Cancel")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
                onClicked: deleteSheet.closeDialog()
            }
        }
    }
}
