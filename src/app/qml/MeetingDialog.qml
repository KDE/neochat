// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-only

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

Kirigami.PromptDialog {
    id: root

    required property bool hasExistingMeeting

    title: hasExistingMeeting ? i18nc("@title", "Join Meeting") : i18nc("@title", "Start Meeting")
    subtitle: hasExistingMeeting ? i18nc("@info:label", "You are about to join a Jitsi meeting in your web browser.") : i18nc("@info:label", "You are about to start a new Jitsi meeting in your web browser.")
    standardButtons: QQC2.Dialog.Cancel

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            icon.name: "camera-video-symbolic"
            text: root.hasExistingMeeting ? i18nc("@action:button Join the Jitsi meeting", "Join") : i18nc("@action:button Start a new Jitsi meeting", "Start")

            onClicked: root.accept()

            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
        }
    }
}
