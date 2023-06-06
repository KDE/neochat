// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

Kirigami.Page {
    id: banSheet

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
                    banSheet.room.ban(banSheet.userId, reason.text)
                    banSheet.closeDialog()
                }
            }
            QQC2.Button {
                text: i18nc("@action", "Cancel")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
                onClicked: banSheet.closeDialog()
            }
        }
    }
}
