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

    title: i18n("Report Message")

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    QQC2.TextArea {
        id: reason
        placeholderText: i18n("Reason for reporting this message")
        anchors.fill: parent
        wrapMode: TextEdit.Wrap

        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
        }
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
