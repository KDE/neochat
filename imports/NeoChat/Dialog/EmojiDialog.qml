/**
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
import QtQuick 2.14
import QtQuick.Controls 2.14 as QQC2
import org.kde.kirigami 2.13 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component.Emoji 1.0

QQC2.Popup {
    id: root

    signal react(string emoji)

    modal: true
    focus: true
    closePolicy: QQC2.Popup.CloseOnEscape | QQC2.Popup.CloseOnPressOutsideParent
    margins: 0
    padding: Kirigami.Units.smallSpacing
    implicitWidth: Kirigami.Units.gridUnit * 15
    implicitHeight: Kirigami.Units.gridUnit * 20

    contentItem: EmojiPicker {
        onChoosen: react(emoji);
        emojiModel: EmojiModel {}
    }
}
