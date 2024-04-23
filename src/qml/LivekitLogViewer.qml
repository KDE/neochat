// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.ApplicationWindow {
    id: root

    title: i18nc("@title", "Livekit logs")

    pageStack.initialPage: Kirigami.ScrollablePage {
        title: i18nc("@title", "Livekit logs")
        TableView {
            id: messageList
            width: root.width
            model: LivekitLogModel
            alternatingRows: true
            delegate: QQC2.ItemDelegate {
                id: messageDelegate

                required property string message
                width: parent.width

                contentItem: QQC2.Label {
                    text: messageDelegate.message
                    wrapMode: QQC2.Label.Wrap
                }
            }
        }
    }
}
