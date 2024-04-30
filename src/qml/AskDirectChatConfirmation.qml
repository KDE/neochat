// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.Dialog {
    id: root

    required property var user

    width: Math.min(Kirigami.Units.gridUnit * 24, QQC2.ApplicationWindow.window.width)
    height: Kirigami.Units.gridUnit * 8

    standardButtons: QQC2.Dialog.Close
    title: i18nc("@title:dialog", "Start a chat")

    contentItem: QQC2.Label {
        text: i18n("Do you want to start a chat with %1?", root.user.displayName)
        textFormat: Text.PlainText
        wrapMode: Text.Wrap
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
    }

    customFooterActions: [
        Kirigami.Action {
            text: i18nc("@action:button", "Start Chat")
            icon.name: "im-user"
            onTriggered: {
                root.user.requestDirectChat();
                root.close();
            }
        }
    ]
}
