// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-only

pragma ComponentBehavior: Bound

import QtQuick

import org.kde.kirigami as Kirigami

Kirigami.PromptDialog {
    id: root

    required property bool hasExistingMeeting

    title: hasExistingMeeting ? i18nc("@title", "Join Meeting") : i18nc("@title", "Start Meeting")
    subtitle: hasExistingMeeting ? i18nc("@info:label", "You are about to join a Jitsi meeting in your web browser.") : i18nc("@info:label", "You are about to start a new Jitsi meeting in your web browser.")
    standardButtons: Kirigami.Dialog.Cancel

    customFooterActions: Kirigami.Action {
        icon.name: "camera-video-symbolic"
        text: root.hasExistingMeeting ? i18nc("@action:button Join the Jitsi meeting", "Join") : i18nc("@action:button Start a new Jitsi meeting", "Start")
        onTriggered: root.accept()
    }
}
