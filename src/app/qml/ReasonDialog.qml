// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.Page {
    id: root

    required property string placeholder
    required property string actionText
    required property string icon

    signal accepted(reason: string)

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    QQC2.TextArea {
        id: reason
        placeholderText: root.placeholder
        anchors.fill: parent
        wrapMode: TextEdit.Wrap
        focus: true

        Keys.onReturnPressed: event => {
            if (event.modifiers & Qt.ControlModifier) {
                root.accepted(reason.text);
                root.closeDialog();
            }
        }

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
                text: root.actionText
                icon.name: root.icon
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
                onClicked: {
                    root.accepted(reason.text);
                    root.closeDialog();
                }
            }
            QQC2.Button {
                icon.name: "dialog-cancel-symbolic"
                text: i18nc("@action", "Cancel")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
                onClicked: root.closeDialog()
            }
        }
    }
}
