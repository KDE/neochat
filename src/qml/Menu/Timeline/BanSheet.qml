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
    property string userId

    title: i18n("Ban User")

    QQC2.TextArea {
        id: reason
        placeholderText: i18n("Reason for banning this user")
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
                text: i18nc("@action:button 'Ban' as in 'Ban this user'", "Ban")
                icon.name: "im-ban-user"
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
                onClicked: {
                    root.room.ban(root.userId, reason.text)
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
