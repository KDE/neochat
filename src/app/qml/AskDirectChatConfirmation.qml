// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.ki18n

Kirigami.Dialog {
    id: root

    required property var user

    width: Math.min(Kirigami.Units.gridUnit * 24, QQC2.ApplicationWindow.window.width)
    height: Kirigami.Units.gridUnit * 8

    standardButtons: QQC2.Dialog.Close
    title: KI18n.i18nc("@title:dialog", "Start a chat")

    contentItem: QQC2.Label {
        text: KI18n.i18nc("@info", "Do you want to start a chat with %1?", root.user.displayName)
        textFormat: Text.PlainText
        wrapMode: Text.Wrap
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
    }

    customFooterActions: [
        Kirigami.Action {
            text: KI18n.i18nc("@action:button", "Start Chat")
            icon.name: "im-user"
            onTriggered: {
                root.user.requestDirectChat();
                root.close();
            }
        }
    ]
}
