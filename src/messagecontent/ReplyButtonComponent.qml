// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.neochat

/**
 * @brief A component to show a reply button for threads in a message bubble.
 */
QQC2.ItemDelegate {
    id: root

    /**
     * @brief The thread root ID.
     */
    required property string threadRoot

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth

    leftInset: 0
    rightInset: 0

    highlighted: true

    icon.name: "mail-reply-custom"
    text: i18nc("@action:button", "Reply")

    onClicked: {
        RoomManager.requestReply("", root.threadRoot);
    }
}
