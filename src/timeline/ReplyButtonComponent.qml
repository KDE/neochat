// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat
import org.kde.neochat.chatbar

/**
 * @brief A component to show a reply button for threads in a message bubble.
 */
Delegates.RoundedItemDelegate {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief The thread root ID.
     */
    required property string threadRoot

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    Layout.fillWidth: true
    Layout.maximumWidth: root.maxContentWidth

    leftInset: 0
    rightInset: 0

    icon.name: "mail-reply-custom"
    text: i18nc("@action:button", "Reply")

    onClicked: {
        root.room.threadCache.replyId = "";
        root.room.threadCache.threadId = root.threadRoot;
        root.room.mainCache.clearRelations();
        root.room.editCache.clearRelations();
    }
}
