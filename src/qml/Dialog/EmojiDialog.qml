// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

QQC2.Popup {
    id: emojiPopup

    /**
     * @brief The current room that user is viewing.
     */
    property NeoChatRoom currentRoom

    property bool includeCustom: false
    property bool closeOnChosen: true
    property bool showQuickReaction: false

    signal chosen(string emoji)

    Connections {
        target: RoomManager
        function onCurrentRoomChanged() {
            emojiPopup.close()
        }
    }

    onVisibleChanged: {
        if (!visible) {
            return
        }
        emojiPicker.forceActiveFocus()
    }

    background: Kirigami.ShadowedRectangle {
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.smallSpacing
        shadow.size: Kirigami.Units.smallSpacing
        shadow.color: Qt.rgba(0.0, 0.0, 0.0, 0.10)
        border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
        border.width: 2
    }

    modal: true
    focus: true
    closePolicy: QQC2.Popup.CloseOnEscape | QQC2.Popup.CloseOnPressOutsideParent
    margins: 0
    padding: 2

    implicitHeight: Kirigami.Units.gridUnit * 20 + 2 * padding
    width: Math.min(contentItem.categoryIconSize * 11 + 2 * padding, QQC2.Overlay.overlay.width)
    contentItem: EmojiPicker {
        id: emojiPicker
        height: 400
        currentRoom: root.currentRoom
        includeCustom: emojiPopup.includeCustom
        showQuickReaction: emojiPopup.showQuickReaction
        onChosen: {
            emojiPopup.chosen(emoji)
            if (emojiPopup.closeOnChosen) emojiPopup.close()
        }
    }
}
