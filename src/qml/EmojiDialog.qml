// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat

QQC2.Popup {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    property NeoChatRoom currentRoom

    property bool closeOnChosen: true

    signal chosen(string emoji)

    Connections {
        target: RoomManager
        function onCurrentRoomChanged() {
            root.close()
        }
    }

    onVisibleChanged: {
        if (!visible) {
            emojiPicker.clearSearchField()
            return
        }
        emojiPicker.forceActiveFocus()
    }

    background: Kirigami.ShadowedRectangle {
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.mediumSpacing
        shadow {
            size: Kirigami.Units.largeSpacing
            color: Qt.rgba(0.0, 0.0, 0.0, 0.3)
            yOffset: 2
        }
        border {
            color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
            width: 2
        }
    }

    modal: true
    focus: true
    clip: false
    closePolicy: QQC2.Popup.CloseOnEscape | QQC2.Popup.CloseOnPressOutsideParent
    margins: 0
    padding: 2

    implicitHeight: Kirigami.Units.gridUnit * 20 + 2 * padding
    width: Math.min(contentItem.implicitWidth + 2 * padding, QQC2.Overlay.overlay.width)

    contentItem: EmojiPicker {
        id: emojiPicker
        height: 400
        currentRoom: root.currentRoom
        onChosen: emoji => {
        onChosen: {
            root.chosen(emoji)
            ImageContentManager.emojiUsed(emoji)
            if (root.closeOnChosen)
                root.close()
            }
        }
    }
}
