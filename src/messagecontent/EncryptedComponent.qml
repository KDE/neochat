// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component for an encrypted message that can't be decrypted.
 */
TextEdit {
    id: root

    /**
     * @brief The Blocks::Block for the delegate; its display says why the message can't be read.
     */
    required property BasicTextBlock block

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth

    text: root.block.display
    color: Kirigami.Theme.disabledTextColor
    selectedTextColor: Kirigami.Theme.highlightedTextColor
    selectionColor: Kirigami.Theme.highlightColor
    font.pointSize: Kirigami.Theme.defaultFont.pointSize
    selectByMouse: !Kirigami.Settings.isMobile
    readOnly: true
    wrapMode: Text.WordWrap
    textFormat: Text.RichText
}
