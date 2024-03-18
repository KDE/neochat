// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

/**
 * @brief A component for an encrypted message that can't be decrypted.
 */
TextEdit {
    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    Layout.fillWidth: true
    Layout.maximumWidth: root.maxContentWidth

    text: i18n("This message is encrypted and the sender has not shared the key with this device.")
    color: Kirigami.Theme.disabledTextColor
    selectedTextColor: Kirigami.Theme.highlightedTextColor
    selectionColor: Kirigami.Theme.highlightColor
    font.pointSize: Kirigami.Theme.defaultFont.pointSize
    selectByMouse: !Kirigami.Settings.isMobile
    readOnly: true
    wrapMode: Text.WordWrap
    textFormat: Text.RichText
}
