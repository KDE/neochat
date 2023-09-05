// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0

Kirigami.Page {
    id: root

    property NeoChatRoom room
    property string eventId

    title: i18n("Report Message")

    QQC2.TextArea {
        id: reason
        placeholderText: i18n("Reason for reporting this message")
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
                text: i18nc("@action:button 'Report' as in 'Report this event to the administrators'", "Report")
                icon.name: "dialog-warning-symbolic"
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
                onClicked: {
                    root.room.reportEvent(eventId, reason.text)
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
